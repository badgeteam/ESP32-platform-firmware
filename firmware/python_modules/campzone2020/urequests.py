import usocket, wifi

class Response:

    def __init__(self, f):
        self.raw = f
        self.encoding = "utf-8"
        self._cached = None

    def close(self):
        if self.raw:
            self.raw.close()
            self.raw = None
        self._cached = None

    @property
    def content(self):
        if self._cached is None:
            self._cached = self.raw.read()
            self.raw.close()
            self.raw = None
        return self._cached

    @property
    def text(self):
        return str(self.content, self.encoding)

    def json(self):
        import ujson
        return ujson.loads(self.content)


def request(method, url, data=None, json=None, headers={}, stream=None, timeout=10, redirect=5):
    if not wifi.status():
        raise ValueError("WiFi not connected")

    try:
        proto, dummy, host, path = url.split("/", 3)
    except ValueError:
        proto, dummy, host = url.split("/", 2)
        path = ""
    if proto == "http:":
        port = 80
    elif proto == "https:":
        import ussl
        port = 443
    else:
        raise ValueError("Unsupported protocol: " + proto)

    if ":" in host:
        host, port = host.split(":", 1)
        port = int(port)

    # Retry a few times as DNS lookup seems to be somewhat unstable
    for _ in range(3):
        ai = usocket.getaddrinfo(host, port)
        if len(ai) > 0:
            break
    if len(ai) == 0:
        raise ValueError("Unable to query DNS record for %s" % host)

    addr = ai[0][-1]
    s = usocket.socket()
    if timeout:
        s.settimeout(timeout)
        s.connect(addr)
    if proto == "https:":
        s = ussl.wrap_socket(s, server_hostname=host)
    s.write(b"%s /%s HTTP/1.0\r\n" % (method, path))
    if not "Host" in headers:
        s.write(b"Host: %s\r\n" % host)
    # Iterate over keys to avoid tuple alloc
    for k in headers:
        s.write(k)
        s.write(b": ")
        s.write(headers[k])
        s.write(b"\r\n")
    if json is not None:
        assert data is None
        import ujson
        data = ujson.dumps(json)
    if data:
        s.write(b"Content-Length: %d\r\n" % len(data))
    s.write(b"\r\n")
    if data:
        s.write(data)

    l = s.readline()
    protover, status, msg = l.split(None, 2)
    status = int(status)
    #print(protover, status, msg)
    while True:
        l = s.readline()
        if not l or l == b"\r\n":
            break
        #print(l)
        if l.lower().startswith(b"transfer-encoding:"):
            if b"chunked" in l:
                s.close()
                raise ValueError("Unsupported " + l)
        elif l.lower().startswith(b"location:") and 300 <= status <= 399:
            s.close()
            if redirect <= 0:
                raise ValueError("Too many redirects")
            else:
                return request(method, l[9:len(l)].decode().strip(), data, json, headers, stream, timeout, redirect-1)

    resp = Response(s)
    resp.status_code = status
    resp.reason = msg.rstrip()
    return resp


def head(url, **kw):
    return request("HEAD", url, **kw)

def get(url, **kw):
    return request("GET", url, **kw)

def post(url, **kw):
    return request("POST", url, **kw)

def put(url, **kw):
    return request("PUT", url, **kw)

def patch(url, **kw):
    return request("PATCH", url, **kw)

def delete(url, **kw):
    return request("DELETE", url, **kw)
