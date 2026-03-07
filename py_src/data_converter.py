import argparse
import os
import struct
from collections import Counter, defaultdict, deque
from scapy.all import Ether, IP, Raw, UDP
from scapy.utils import PcapWriter

# --- CONFIGURATION ---
INPUT_FILENAME = "market_data/12302019.NASDAQ_ITCH50"  # Your downloaded raw file
OUTPUT_FILENAME = "market_data/nasdaq_orderbook_AAPL.pcap"
FRAME_LIMIT = 1  # How many packets you want (set to 0 for all)
FRAME_TYPES_PER_FRAME = [ "F","X", "C", "E",  "A", "U", "D"]  # Ordered ITCH message types in each frame

# If non-empty, generate one PCAP per symbol (e.g. ["AAPL", "TSLA"]) and only include
# messages for those symbols. If empty, keep the original behavior (single output PCAP).
FILTER_SYMBOLS = ["AAPL"]  # Example: ["AAPL", "TSLA"]
FILTER_ORDER_REF_NUM = 11825       # Example: 123456789 (or None to disable)
FILTER_NEW_ORDER_REF_NUM = None   # Example: 987654321 (or None to disable)

# Input example ./dataconverter.py -t A F X U D E
# Network Config
SRC_MAC = "00:11:22:33:44:55"
DST_MAC = "01:00:5E:00:00:01"
SRC_IP  = "192.168.1.50"
DST_IP  = "233.1.2.3"
DST_PORT = 1234

def _parse_type_sequence(raw_types):
    type_sequence = []
    for item in raw_types:
        for token in item.split(","):
            token = token.strip().upper()
            if len(token) != 1:
                continue
            try:
                type_sequence.append(token.encode("ascii"))
            except UnicodeEncodeError:
                continue
    return type_sequence


def _normalize_symbol(symbol):
    if symbol is None:
        return None
    return symbol.strip().upper()


def _parse_optional_u64(value):
    if value is None:
        return None
    if isinstance(value, int):
        parsed = value
    else:
        text = str(value).strip()
        if text == "":
            return None
        parsed = int(text, 0)  # supports decimal and 0x... hex
    if parsed < 0 or parsed > 0xFFFFFFFFFFFFFFFF:
        raise ValueError(f"value out of uint64 range: {value}")
    return parsed


def _extract_order_refs(itch_payload):
    msg_type = itch_payload[0:1]
    order_ref = None
    new_order_ref = None

    try:
        if msg_type in (b"A", b"F", b"X", b"D", b"E"):
            if len(itch_payload) >= 19:
                order_ref = struct.unpack(">Q", itch_payload[11:19])[0]
        elif msg_type == b"U":
            if len(itch_payload) >= 27:
                order_ref = struct.unpack(">Q", itch_payload[11:19])[0]
                new_order_ref = struct.unpack(">Q", itch_payload[19:27])[0]
    except struct.error:
        return None, None

    return order_ref, new_order_ref


def _extract_symbol_and_update_maps(
    itch_payload,
    stock_locate_to_symbol,
    order_ref_to_symbol,
    requested_symbols_set=None,
):
    msg_type = itch_payload[0:1]

    # Offsets for ITCH 5.0 order book related messages (all big-endian for ints):
    # All of these start with:
    #   0: type (1)
    #   1: stock locate (2)
    #   3: tracking number (2)
    #   5: timestamp (6)
    #
    # A: ... order_ref(8) buy/sell(1) shares(4) stock(8) price(4)
    # F: A + attribution(4)
    # X: ... order_ref(8) canceled_shares(4)
    # D: ... order_ref(8)
    # U: ... orig_ref(8) new_ref(8) shares(4) price(4)
    # E: ... order_ref(8) executed_shares(4) match_num(8)
    #
    # R (stock directory): ... stock(8) ...
    try:
        stock_locate = struct.unpack(">H", itch_payload[1:3])[0]
    except struct.error:
        return None

    symbol = stock_locate_to_symbol.get(stock_locate)

    if msg_type == b"R":
        if len(itch_payload) >= 19:
            stock = itch_payload[11:19].decode("ascii", errors="ignore").strip()
            stock_norm = _normalize_symbol(stock)
            if stock_norm and (not requested_symbols_set or stock_norm in requested_symbols_set):
                stock_locate_to_symbol[stock_locate] = stock_norm
        return None

    if msg_type in (b"A", b"F"):
        if len(itch_payload) < (36 if msg_type == b"A" else 40):
            return symbol
        order_ref = struct.unpack(">Q", itch_payload[11:19])[0]
        stock = itch_payload[24:32].decode("ascii", errors="ignore").strip()
        stock_norm = _normalize_symbol(stock)
        if stock_norm:
            if not requested_symbols_set or stock_norm in requested_symbols_set:
                order_ref_to_symbol[order_ref] = stock_norm
                stock_locate_to_symbol.setdefault(stock_locate, stock_norm)
            return stock_norm
        return symbol

    if msg_type == b"U":
        if len(itch_payload) < 35:
            return symbol
        orig_ref = struct.unpack(">Q", itch_payload[11:19])[0]
        new_ref = struct.unpack(">Q", itch_payload[19:27])[0]
        sym = stock_locate_to_symbol.get(stock_locate) or order_ref_to_symbol.get(orig_ref)
        sym_norm = _normalize_symbol(sym)
        if sym_norm:
            if not requested_symbols_set or sym_norm in requested_symbols_set:
                order_ref_to_symbol[new_ref] = sym_norm
        return sym_norm

    if msg_type in (b"X", b"D", b"E"):
        if len(itch_payload) < 19:
            return symbol
        order_ref = struct.unpack(">Q", itch_payload[11:19])[0]
        return _normalize_symbol(stock_locate_to_symbol.get(stock_locate) or order_ref_to_symbol.get(order_ref))

    return _normalize_symbol(symbol)


def filter_by_template(
    input_filename,
    output_filename,
    frame_limit,
    frame_types_per_frame,
    append_output=False,
    symbols=None,
    order_ref_filter=None,
    new_order_ref_filter=None,
):
    if not frame_types_per_frame:
        raise ValueError("frame_types_per_frame must not be empty")

    requested_symbols = [_normalize_symbol(s) for s in (symbols or [])]
    requested_symbols = [s for s in requested_symbols if s]
    requested_symbols_set = set(requested_symbols)

    selected = ", ".join(sorted({t.decode("ascii") for t in frame_types_per_frame}))
    template = ",".join(t.decode("ascii") for t in frame_types_per_frame)
    print(f"Filtering {input_filename} for types: {selected}")
    mode = "append" if append_output else "overwrite"
    if requested_symbols_set:
        print(f"Output mode: {mode} ({output_filename} -> one file per symbol)")
        print(f"Symbol filter: {', '.join(sorted(requested_symbols_set))}")
    else:
        print(f"Output mode: {mode} ({output_filename})")
    print(f"Frame template: [{template}]")
    if order_ref_filter is not None:
        print(f"Order-ref filter: {order_ref_filter}")
    if new_order_ref_filter is not None:
        print(f"New-order-ref filter: {new_order_ref_filter}")

    required_counts = Counter(frame_types_per_frame)

    stock_locate_to_symbol = {}
    order_ref_to_symbol = {}

    def _make_symbol_output_path(symbol):
        base, ext = os.path.splitext(output_filename)
        ext = ext or ".pcap"
        return f"{base}_{symbol}{ext}"

    def _open_writer(path):
        return PcapWriter(path, append=append_output, sync=True)

    # per-output state
    writers = {}  # key -> PcapWriter
    seq_num_by_key = defaultdict(lambda: 1)
    saved_count_by_key = defaultdict(int)
    type_counts_by_key = defaultdict(lambda: defaultdict(int))
    buffered_by_key = defaultdict(lambda: defaultdict(deque))
    seen_msgs_by_symbol = defaultdict(int)
    done_symbols = set()

    stop_all = False

    def can_build_frame(buffered_by_type):
        for msg_type, req_count in required_counts.items():
            if len(buffered_by_type[msg_type]) < req_count:
                return False
        return True

    def flush_frame(key, buffered_by_type):
        frame_blocks = []
        for msg_type in frame_types_per_frame:
            frame_blocks.append(buffered_by_type[msg_type].popleft())

        msg_count = len(frame_types_per_frame)
        seq_num = seq_num_by_key[key]
        mold_header = struct.pack(">10sQH", b"NASDQTEST ", seq_num, msg_count)
        pkt = (
            Ether(src=SRC_MAC, dst=DST_MAC)
            / IP(src=SRC_IP, dst=DST_IP)
            / UDP(sport=5555, dport=DST_PORT)
            / Raw(load=mold_header + b"".join(frame_blocks))
        )

        if key not in writers:
            out_path = _make_symbol_output_path(key) if requested_symbols_set else output_filename
            writers[key] = _open_writer(out_path)
        writers[key].write(pkt)
        seq_num_by_key[key] = seq_num + msg_count
        saved_count_by_key[key] += 1

    if not requested_symbols_set:
        writers["__ALL__"] = _open_writer(output_filename)

    with open(input_filename, "rb") as f:
        while True:
            # 1. Read Message Length (2 bytes)
            len_bytes = f.read(2)
            if not len_bytes:
                break
            msg_len = struct.unpack(">H", len_bytes)[0]

            # 2. Read Message Body
            itch_payload = f.read(msg_len)
            if len(itch_payload) < msg_len:
                break

            msg_type = itch_payload[0:1]

            sym = None
            if requested_symbols_set:
                # Keep maps warm so symbol filtering works even for messages without an explicit stock symbol.
                sym = _extract_symbol_and_update_maps(
                    itch_payload,
                    stock_locate_to_symbol,
                    order_ref_to_symbol,
                    requested_symbols_set=requested_symbols_set,
                )
                sym = _normalize_symbol(sym)
                if sym in requested_symbols_set:
                    seen_msgs_by_symbol[sym] += 1

            if requested_symbols_set and done_symbols == requested_symbols_set:
                break

            if msg_type not in required_counts:
                continue

            if order_ref_filter is not None or new_order_ref_filter is not None:
                msg_order_ref, msg_new_order_ref = _extract_order_refs(itch_payload)
                if order_ref_filter is not None and msg_order_ref != order_ref_filter:
                    continue
                if new_order_ref_filter is not None and msg_new_order_ref != new_order_ref_filter:
                    continue

            if requested_symbols_set:
                if not sym or sym not in requested_symbols_set:
                    continue
                if sym in done_symbols:
                    continue
                key = sym
            else:
                key = "__ALL__"

            msg_block = struct.pack(">H", msg_len) + itch_payload
            buffered_by_key[key][msg_type].append(msg_block)
            type_counts_by_key[key][msg_type] += 1

            while can_build_frame(buffered_by_key[key]):
                flush_frame(key, buffered_by_key[key])

                saved_count = saved_count_by_key[key]
                if saved_count % 1000 == 0:
                    prefix = f"[{key}] " if requested_symbols_set else ""
                    print(f"{prefix}Saved {saved_count} packets...", end="\r")

                if frame_limit > 0 and saved_count >= frame_limit:
                    if requested_symbols_set:
                        done_symbols.add(key)
                        break
                    print(f"\nLimit reached ({frame_limit}). Stopping.")
                    stop_all = True
                    break
            if stop_all:
                break

    for writer in writers.values():
        writer.close()

    if requested_symbols_set:
        print("\nDone!")
        for sym in sorted(requested_symbols_set):
            out_path = _make_symbol_output_path(sym)
            saved = saved_count_by_key[sym]
            seen = seen_msgs_by_symbol[sym]
            if seen == 0:
                print(f"Symbol '{sym}' not found in input. Skipping.")
            elif saved == 0:
                print(f"Symbol '{sym}' had messages but could not build any full frames. Skipping output.")
            else:
                print(f"Saved {saved} packets to {out_path}")
    else:
        print(f"\nDone! Saved to {output_filename}")
        for order_type in sorted(required_counts):
            letter = order_type.decode("ascii")
            print(f"Type '{letter}': {type_counts_by_key['__ALL__'][order_type]} message(s)")


def main():
    parser = argparse.ArgumentParser(description="Extract ITCH messages into PCAP frames using a type template.")
    parser.add_argument(
        "-t",
        "--frame-types",
        nargs="+",
        help="Ordered message types in each frame. Example: -t A,F,E",
    )
    parser.add_argument("--input", default=INPUT_FILENAME, help=f"Input ITCH file (default: {INPUT_FILENAME})")
    parser.add_argument("--output", default=OUTPUT_FILENAME, help=f"Output PCAP file (default: {OUTPUT_FILENAME})")
    parser.add_argument("--limit", type=int, default=FRAME_LIMIT, help=f"Packet limit, 0 means all (default: {FRAME_LIMIT})")
    parser.add_argument(
        "--order-ref",
        default=FILTER_ORDER_REF_NUM,
        help=f"Optional order reference number filter (uint64, decimal or 0x..). Default: {FILTER_ORDER_REF_NUM}",
    )
    parser.add_argument(
        "--new-order-ref",
        default=FILTER_NEW_ORDER_REF_NUM,
        help=f"Optional new order reference number filter (uint64, decimal or 0x..). Default: {FILTER_NEW_ORDER_REF_NUM}",
    )
    parser.add_argument(
        "--append",
        action="store_true",
        help="Append to existing output PCAP instead of overwriting it",
    )
    args = parser.parse_args()

    raw_types = args.frame_types
    if not raw_types:
        raw = input(f"Enter frame template (e.g. A,F,E) [default: {','.join(FRAME_TYPES_PER_FRAME)}]: ").strip()
        if raw:
            raw_types = [raw]
        else:
            raw_types = [",".join(FRAME_TYPES_PER_FRAME)]

    frame_types_per_frame = _parse_type_sequence(raw_types)
    if not frame_types_per_frame:
        print("No valid frame template provided. Nothing to do.")
        return

    try:
        order_ref_filter = _parse_optional_u64(args.order_ref)
        new_order_ref_filter = _parse_optional_u64(args.new_order_ref)
    except ValueError as e:
        print(f"Invalid reference filter: {e}")
        return

    filter_by_template(
        args.input,
        args.output,
        args.limit,
        frame_types_per_frame,
        append_output=args.append,
        symbols=FILTER_SYMBOLS,
        order_ref_filter=order_ref_filter,
        new_order_ref_filter=new_order_ref_filter,
    )

if __name__ == "__main__":
    main()
