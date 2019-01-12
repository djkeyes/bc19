
import base64


# Python 2-3 compatibility helper function:
# Converts a string to the native str type.
def asstr(s):
  if str is bytes:
    if isinstance(s, unicode):
      return s.encode('utf-8')
  elif isinstance(s, bytes):
    return s.decode('utf-8')
  return s

path = 'hello3.wasm'
with open(path, 'rb') as f:
  data = base64.b64encode(f.read())
result = 'data:application/octet-stream;base64,' + asstr(data)
print(result)