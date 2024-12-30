#!/usr/bin/env python3
import sys
import argparse
import json
import codecs
import datetime

from cereal import log
from cereal.services import SERVICE_LIST
from openpilot.tools.lib.live_logreader import raw_live_logreader

codecs.register_error("strict", codecs.backslashreplace_errors)

def hexdump(msg):
  m = str.upper(msg.hex())
  m = [m[i:i + 2] for i in range(0, len(m), 2)]
  m = [m[i:i + 16] for i in range(0, len(m), 16)]
  lines = []
  for row, dump in enumerate(m):
    addr = '%08X:' % (row * 16)
    raw = ' '.join(dump[:8]) + '  ' + ' '.join(dump[8:])
    space = ' ' * (48 - len(raw))
    asci = ''.join(chr(int(x, 16)) if 0x20 <= int(x, 16) <= 0x7E else '.' for x in dump)
    lines.append(f"{addr} {raw} {space} {asci}")
  return "\n".join(lines)

def format_current_time():
  return datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")

if __name__ == "__main__":
  parser = argparse.ArgumentParser(description="Dump communication sockets.")
  parser.add_argument('--pipe', action='store_true', help='Raw byte stream output')
  parser.add_argument('--raw', action='store_true', help='Hexadecimal dump output')
  parser.add_argument('--json', action='store_true', help='Output each event as JSON format')
  parser.add_argument('--addr', default='127.0.0.1', help='Address to listen to (default: 127.0.0.1)')
  parser.add_argument('-v', '--values', help='Specific variables to monitor, e.g., param.name')
  parser.add_argument('-c', '--count', type=int, help='Number of iterations to run before exiting')
  parser.add_argument('-o', '--output', help='Output file')
  parser.add_argument("socket", type=str, nargs='*', default=list(SERVICE_LIST.keys()),
                      help="Socket names to dump. Defaults to all sockets defined in cereal.")
  args = parser.parse_args()

  lr = raw_live_logreader(args.socket, args.addr)
  values = [s.strip().split(".") for s in args.values.split(",")] if args.values else None
  count = args.count if args.count else sys.maxsize
  iterations = 0

  output_file = open(args.output, 'w') if args.output else None

  # Print initial separator
  initial_separator = f"{'-' * 80}\n    Dump communication sockets (1/{count}): {', '.join(args.socket)}\n{'-' * 80}\n"
  if output_file:
    output_file.write(initial_separator)
  else:
    sys.stdout.write(initial_separator)
    sys.stdout.flush()

  try:
    for msg in lr:
      try:
        if iterations >= count:
          break

        with log.Event.from_bytes(msg) as evt:
          iterations += 1

          output_lines = []
          if args.pipe:
            output_lines.append(msg.decode('utf-8') + "\n")
          elif args.raw:
            output_lines.append(hexdump(msg) + "\n")
          elif args.json:
            try:
              json_data = json.dumps(evt.to_dict(), default=str)
              output_lines.append(json_data + "\n")
            except TypeError as e:
              print(f"JSON serialization error: {e}, event type: {type(evt)}")
              continue
          elif values:
            for value in values:
              item = evt
              for key in value:
                item = getattr(item, key, None)
              if item is not None:
                output_lines.append(f"{'.'.join(value)} = {item}\n")
          else:
            evt_str = str(evt).replace("logMonoTime", f"currentTime ({format_current_time()})")
            output_lines.append(evt_str + "\n")

          if output_file:
            output_file.writelines(output_lines)
          else:
            sys.stdout.writelines(output_lines)
            sys.stdout.flush()

      except Exception as e:
        print(f"Error processing message: {e}")
        continue

  except KeyboardInterrupt:
    print("Exiting...")

  finally:
    if output_file:
      output_file.close()
