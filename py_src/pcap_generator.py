#!/usr/bin/env python3
import argparse
import os
import re
import struct
from dataclasses import dataclass
from decimal import Decimal, ROUND_HALF_UP
from pathlib import Path

DEFAULT_DST_MAC = "01:00:5E:00:00:01"
DEFAULT_SRC_MAC = "00:11:22:33:44:55"
DEFAULT_DST_IP = "233.1.2.3"
DEFAULT_SRC_IP = "192.168.1.50"
DEFAULT_DST_PORT = 1234
DEFAULT_SRC_PORT = 5555


def _default_hex_output_path(output_filename):
    base, ext = os.path.splitext(output_filename)
    if not ext:
        return f"{output_filename}_frames_hex.txt"
    return f"{base}_frames_hex.txt"


def _format_frame_as_tb_lines(frame_bytes):
    def _hx(chunk):
        return "_".join(f"{b:02x}" for b in chunk)

    lines = []
    offset = 0
    total = len(frame_bytes)

    while offset + 16 <= total:
        lines.append(f"set16('h{offset:04x}, 128'h{_hx(frame_bytes[offset:offset + 16])});")
        offset += 16

    if offset + 8 <= total:
        lines.append(f"set8 ('h{offset:04x}, 64'h{_hx(frame_bytes[offset:offset + 8])});")
        offset += 8

    while offset + 2 <= total:
        lines.append(f"set2 ('h{offset:04x}, 16'h{_hx(frame_bytes[offset:offset + 2])});")
        offset += 2

    if offset < total:
        lines.append(f"frame_bytes['h{offset:04x}] = 8'h{frame_bytes[offset]:02x};")

    return lines


def _ensure_uint(value, bits, field_name):
    max_val = (1 << bits) - 1
    if value < 0 or value > max_val:
        raise ValueError(f"{field_name} must fit in uint{bits}, got {value}")
    return value


def _pack_u48(value, field_name):
    _ensure_uint(value, 48, field_name)
    return value.to_bytes(6, byteorder="big", signed=False)


def _pack_ascii(value, width, field_name):
    try:
        encoded = value.encode("ascii")
    except UnicodeEncodeError as exc:
        raise ValueError(f"{field_name} must be ASCII: {value!r}") from exc
    if len(encoded) > width:
        raise ValueError(f"{field_name} too long (max {width}): {value!r}")
    return encoded.ljust(width, b" ")


def _pack_price_4(price):
    # ITCH price fields are integer with 4 implied decimal places.
    if isinstance(price, int):
        price_int = price
    else:
        price_int = int((Decimal(str(price)) * Decimal("10000")).to_integral_value(rounding=ROUND_HALF_UP))
    _ensure_uint(price_int, 32, "price")
    return struct.pack(">I", price_int)


class ItchMessage:
    def to_itch_payload(self):
        raise NotImplementedError

    def to_mold_block(self):
        payload = self.to_itch_payload()
        return struct.pack(">H", len(payload)) + payload


@dataclass(frozen=True)
class _ItchBookMessage(ItchMessage):
    stock_locate: int
    tracking_number: int
    timestamp: int

    def _pack_common(self):
        _ensure_uint(self.stock_locate, 16, "stock_locate")
        _ensure_uint(self.tracking_number, 16, "tracking_number")
        return struct.pack(">HH", self.stock_locate, self.tracking_number) + _pack_u48(self.timestamp, "timestamp")


@dataclass(frozen=True)
class TYPE_A(_ItchBookMessage):
    order_reference_number: int
    buy_sell_indicator: str
    shares: int
    stock: str
    price: Decimal | int | float | str

    def to_itch_payload(self):
        _ensure_uint(self.order_reference_number, 64, "order_reference_number")
        _ensure_uint(self.shares, 32, "shares")
        side = _pack_ascii(self.buy_sell_indicator, 1, "buy_sell_indicator")
        return (
            b"A"
            + self._pack_common()
            + struct.pack(">Q", self.order_reference_number)
            + side
            + struct.pack(">I", self.shares)
            + _pack_ascii(self.stock, 8, "stock")
            + _pack_price_4(self.price)
        )


@dataclass(frozen=True)
class TYPE_F(_ItchBookMessage):
    order_reference_number: int
    buy_sell_indicator: str
    shares: int
    stock: str
    price: Decimal | int | float | str
    attribution: str

    def to_itch_payload(self):
        _ensure_uint(self.order_reference_number, 64, "order_reference_number")
        _ensure_uint(self.shares, 32, "shares")
        side = _pack_ascii(self.buy_sell_indicator, 1, "buy_sell_indicator")
        return (
            b"F"
            + self._pack_common()
            + struct.pack(">Q", self.order_reference_number)
            + side
            + struct.pack(">I", self.shares)
            + _pack_ascii(self.stock, 8, "stock")
            + _pack_price_4(self.price)
            + _pack_ascii(self.attribution, 4, "attribution")
        )


@dataclass(frozen=True)
class TYPE_X(_ItchBookMessage):
    order_reference_number: int
    canceled_shares: int

    def to_itch_payload(self):
        _ensure_uint(self.order_reference_number, 64, "order_reference_number")
        _ensure_uint(self.canceled_shares, 32, "canceled_shares")
        return (
            b"X"
            + self._pack_common()
            + struct.pack(">Q", self.order_reference_number)
            + struct.pack(">I", self.canceled_shares)
        )


@dataclass(frozen=True)
class TYPE_C(_ItchBookMessage):
    order_reference_number: int
    executed_shares: int
    match_number: int
    printable: str
    execution_price: Decimal | int | float | str

    def to_itch_payload(self):
        _ensure_uint(self.order_reference_number, 64, "order_reference_number")
        _ensure_uint(self.executed_shares, 32, "executed_shares")
        _ensure_uint(self.match_number, 64, "match_number")
        return (
            b"C"
            + self._pack_common()
            + struct.pack(">Q", self.order_reference_number)
            + struct.pack(">I", self.executed_shares)
            + struct.pack(">Q", self.match_number)
            + _pack_ascii(self.printable, 1, "printable")
            + _pack_price_4(self.execution_price)
        )


@dataclass(frozen=True)
class TYPE_E(_ItchBookMessage):
    order_reference_number: int
    executed_shares: int
    match_number: int

    def to_itch_payload(self):
        _ensure_uint(self.order_reference_number, 64, "order_reference_number")
        _ensure_uint(self.executed_shares, 32, "executed_shares")
        _ensure_uint(self.match_number, 64, "match_number")
        return (
            b"E"
            + self._pack_common()
            + struct.pack(">Q", self.order_reference_number)
            + struct.pack(">I", self.executed_shares)
            + struct.pack(">Q", self.match_number)
        )


@dataclass(frozen=True)
class TYPE_U(_ItchBookMessage):
    original_order_reference_number: int
    new_order_reference_number: int
    shares: int
    price: Decimal | int | float | str

    def to_itch_payload(self):
        _ensure_uint(self.original_order_reference_number, 64, "original_order_reference_number")
        _ensure_uint(self.new_order_reference_number, 64, "new_order_reference_number")
        _ensure_uint(self.shares, 32, "shares")
        return (
            b"U"
            + self._pack_common()
            + struct.pack(">Q", self.original_order_reference_number)
            + struct.pack(">Q", self.new_order_reference_number)
            + struct.pack(">I", self.shares)
            + _pack_price_4(self.price)
        )


@dataclass(frozen=True)
class TYPE_D(_ItchBookMessage):
    order_reference_number: int

    def to_itch_payload(self):
        _ensure_uint(self.order_reference_number, 64, "order_reference_number")
        return b"D" + self._pack_common() + struct.pack(">Q", self.order_reference_number)


@dataclass(frozen=True)
class MoldUDP64Frame:
    session: str
    sequence_number: int
    messages: list[ItchMessage]

    def to_payload(self):
        _ensure_uint(self.sequence_number, 64, "sequence_number")
        msg_count = len(self.messages)
        _ensure_uint(msg_count, 16, "message_count")
        header = struct.pack(">10sQH", _pack_ascii(self.session, 10, "session"), self.sequence_number, msg_count)
        return header + b"".join(msg.to_mold_block() for msg in self.messages)


# Structured manual payload templates.
# Edit these message fields directly instead of editing large hex strings.
MANUAL_MESSAGE_FRAMES = [
    MoldUDP64Frame(
        session="NASDQTEST",
        sequence_number=1,
        messages=[
            TYPE_A(
                stock_locate=13,
                tracking_number=0,
                timestamp=14400007075802,
                order_reference_number=1,
                buy_sell_indicator="B",
                shares=10,
                stock="AAPL",
                price="000.01",
            ),
            TYPE_F(
                stock_locate=13,
                tracking_number=0,
                timestamp=30607343087807,
                order_reference_number=2,
                buy_sell_indicator="B",
                shares=500,
                stock="AAPL",
                price="000.02",
                attribution="TSSM",
            ),
            TYPE_X(
                stock_locate=13,
                tracking_number=0,
                timestamp=29887725030607,
                order_reference_number=1,
                canceled_shares=10,
            ),
            TYPE_X(
                stock_locate=13,
                tracking_number=0,
                timestamp=29887725030607,
                order_reference_number=2,
                canceled_shares=100,
            ),
            TYPE_C(
                stock_locate=13,
                tracking_number=1,
                timestamp=34200776180089,
                order_reference_number=2,
                executed_shares=200,
                match_number=166998,
                printable="N",
                execution_price="000.02",
            ),
            TYPE_E(
                stock_locate=13,
                tracking_number=2,
                timestamp=14788203892840,
                order_reference_number=2,
                executed_shares=200,
                match_number=17959,
            ),
            TYPE_A(
                stock_locate=13,
                tracking_number=0,
                timestamp=14400007075802,
                order_reference_number=3,
                buy_sell_indicator="B",
                shares=20,
                stock="AAPL",
                price="000.03",
            ),
            TYPE_U(
                stock_locate=13,
                tracking_number=0,
                timestamp=33631366330025,
                original_order_reference_number=3,
                new_order_reference_number=4,
                shares=100,
                price="000.04",
            ),
            TYPE_D(
                stock_locate=13,
                tracking_number=0,
                timestamp=14433223196496,
                order_reference_number=4,
            ),
        ],
    )
]

# Optional raw-hex fallback.
MANUAL_HEX_PAYLOADS = [frame.to_payload().hex() for frame in MANUAL_MESSAGE_FRAMES]


def _hex_to_bytes(hex_text):
    # Keep only hexadecimal characters so pasted multi-line dumps are accepted.
    cleaned = re.sub(r"[^0-9a-fA-F]", "", hex_text)
    if not cleaned:
        raise ValueError("empty payload")
    if len(cleaned) % 2 != 0:
        raise ValueError(f"hex length must be even, got {len(cleaned)}")
    return bytes.fromhex(cleaned)


def _load_payloads(payload_args, payload_file):
    payloads = []

    for item in payload_args:
        payloads.append(_hex_to_bytes(item))

    if payload_file:
        path = Path(payload_file)
        if not path.exists():
            raise FileNotFoundError(f"payload file not found: {payload_file}")
        for line_no, line in enumerate(path.read_text().splitlines(), start=1):
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            try:
                payloads.append(_hex_to_bytes(line))
            except ValueError as exc:
                raise ValueError(f"invalid hex at {payload_file}:{line_no}: {exc}") from exc

    if not payloads and MANUAL_MESSAGE_FRAMES:
        for idx, frame in enumerate(MANUAL_MESSAGE_FRAMES, start=1):
            try:
                payloads.append(frame.to_payload())
            except ValueError as exc:
                raise ValueError(f"invalid structured frame in MANUAL_MESSAGE_FRAMES[{idx}]: {exc}") from exc

    if not payloads and MANUAL_HEX_PAYLOADS:
        for idx, item in enumerate(MANUAL_HEX_PAYLOADS, start=1):
            try:
                payloads.append(_hex_to_bytes(item))
            except ValueError as exc:
                raise ValueError(f"invalid hex in MANUAL_HEX_PAYLOADS[{idx}]: {exc}") from exc

    return payloads


def main():
    from scapy.all import Ether, IP, Raw, UDP, wrpcap

    parser = argparse.ArgumentParser(
        description="Generate a PCAP from manually provided hex payload(s)."
    )
    parser.add_argument(
        "--payload",
        action="append",
        default=[],
        help="Hex payload string. Repeat this argument to add multiple packets.",
    )
    parser.add_argument(
        "--payload-file",
        help="Text file with one hex payload per line (# comments allowed).",
    )
    parser.add_argument("--output", default="market_data/manual_payload.pcap")
    parser.add_argument(
        "--hex-output",
        default=None,
        help="Output text file for full frame hex in testbench set16/set8/set2 format. "
             "Default: <output>_frames_hex.txt",
    )

    parser.add_argument("--src-mac", default=DEFAULT_SRC_MAC)
    parser.add_argument("--dst-mac", default=DEFAULT_DST_MAC)
    parser.add_argument("--src-ip", default=DEFAULT_SRC_IP)
    parser.add_argument("--dst-ip", default=DEFAULT_DST_IP)
    parser.add_argument("--src-port", type=int, default=DEFAULT_SRC_PORT)
    parser.add_argument("--dst-port", type=int, default=DEFAULT_DST_PORT)
    args = parser.parse_args()

    payloads = _load_payloads(args.payload, args.payload_file)
    if not payloads:
        parser.error(
            "Provide payload(s) via --payload/--payload-file or set MANUAL_MESSAGE_FRAMES/MANUAL_HEX_PAYLOADS in script"
        )

    packet_entries = []
    for payload in payloads:
        pkt = (
            Ether(src=args.src_mac, dst=args.dst_mac)
            / IP(src=args.src_ip, dst=args.dst_ip)
            / UDP(sport=args.src_port, dport=args.dst_port)
            / Raw(load=payload)
        )
        packet_entries.append((pkt, payload))

    packets = [pkt for pkt, _ in packet_entries]
    hex_output_path = args.hex_output or _default_hex_output_path(args.output)

    wrpcap(args.output, packets)
    with open(hex_output_path, "w", encoding="ascii") as f:
        for idx, (pkt, payload) in enumerate(packet_entries, start=1):
            raw = bytes(pkt)
            if len(payload) >= 20:
                seq_num = int.from_bytes(payload[10:18], byteorder="big", signed=False)
                msg_count = int.from_bytes(payload[18:20], byteorder="big", signed=False)
                f.write(f"// frame {idx} bytes={len(raw)} seq={seq_num} msg_count={msg_count}\n")
            else:
                f.write(f"// frame {idx} bytes={len(raw)}\n")
            for line in _format_frame_as_tb_lines(raw):
                f.write(f"{line}\n")
            f.write("\n")

    print(f"Wrote {len(packets)} packet(s) to {args.output}")
    print(f"Wrote {len(packets)} frame hex block(s) to {hex_output_path}")


if __name__ == "__main__":
    main()
