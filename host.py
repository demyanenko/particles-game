import http.server
import socketserver
import sys

port = int(sys.argv[1])

Handler = http.server.SimpleHTTPRequestHandler
Handler.extensions_map = {
    '.html': 'text/html',
    '.js':	'application/x-javascript',
    '.wasm': 'application/wasm',
	'': 'application/octet-stream'
}

with socketserver.TCPServer(("", port), Handler) as httpd:
    print("serving at port", port)
    httpd.serve_forever()