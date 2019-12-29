# onset-http-plugin
Authors: Digital

### Features
* Sync.
* GET, POST, PUT, HEAD, DELETE & PATCH requests.
* No JVM this time! :)

More features such as Async and other HTTP request methods are coming soon.

### Installation
1. Download onset-http-plugin.dll (Windows) & onset-http-plugin.so (Linux) from Releases ([HERE](https://github.com/dig/onset-http-plugin/releases)) and place inside plugins folder.
1. Enable "onset-http-plugin" as a plugin inside server_config.json.

### Building the plugin (Interested in contributing)
#### Windows
1. Install vcpkg (c++ package manager), instructions [here](https://docs.microsoft.com/en-us/cpp/build/vcpkg?view=vs-2019).
1. Install cURL static libraries. `./vcpkg install curl:x64-windows-static`
1. Integrate into VS Studio. `./vcpkg integrate install`
1. Run cmake in root project directory. `cmake -DCMAKE_GENERATOR_PLATFORM=x64 . -DVCPKG_TARGET_TRIPLET=x64-windows-static`
1. Open onset-http-plugin.sln in Visual Studio.
1. Change to Release and click Rebuild under `Build > Rebuild Solution`.
1. DLL will be `src/Release/onset-http-plugin.dll`

#### Linux
1. Install Libcurl4 and Libcurl-openssl-dev. `sudo apt-get install libcurl4 libcurl4-openssl-dev -y`
1. Run cmake in root project directory. `cmake .`
1. Run make in root project directory. `make`
1. Output will be `src/onset-http-plugin.so`

### Example
```lua
http_set_user_agent("myCustomPlugin/1.0")

-- Synchronously send a get request to http://www.httpbin.org/get and print the body
local _res = http_get("http://www.httpbin.org/get", {
  customHeader = "123",
  authentication = "bearer 123",
})
print (_res.body)

-- Synchronously send a post request to http://www.httpbin.org/post and print the status code
local _res = http_post("http://www.httpbin.org/post", {
  customHeader = "123",
  authentication = "bearer 123",
}, {
  field = "123",
  name = "joseph"
})
print (_res.status)

```

### Functions
#### http_get (Sync)
Send a Get request.
```lua
http_get(url, headers)
http_get(url, headers, params)
```
* **url** The URL to send the request to. Example: https://google.com
* **headers** Table of headers to send. Example: { myHeader = "hi", anotherHeader = "lol", authentication = "bearer 123" }
* **params** Parameters to add to the request. Example: { apiKey = "123" }

Returns a table with body (string), status (int), headers (table) and elapsed (float). Nil if failed.
```lua
{
  status = 200,
  elapsed = 0.123,
  body = "content here",
  headers = {
    ["Host"] = "www.httpbin.org" 
  }
}
```

#### http_post (Sync)
Send a Post request.
```lua
http_post(url, headers, body)
http_post(url, headers, fields)
```
* **url** The URL to send the request to. Example: https://google.com
* **headers** Table of headers to send. Example: { myHeader = "hi", anotherHeader = "lol", authentication = "bearer 123" }
* **body** String body, could be JSON. Example: "Hello, I am the body of the POST request."
* **fields** Same as above but table is automatically parsed to JSON. Example: { key = "123", name = "Joseph" }

Returns a table with body (string), status (int), headers (table) and elapsed (float). Nil if failed.
```lua
{
  status = 200,
  elapsed = 0.123,
  body = "content here",
  headers = {
    ["Host"] = "www.httpbin.org" 
  }
}
```

#### http_put (Sync)
Send a Put request.
```lua
http_put(url, headers, body)
http_put(url, headers, fields)
```
* **url** The URL to send the request to. Example: https://google.com
* **headers** Table of headers to send. Example: { myHeader = "hi", anotherHeader = "lol", authentication = "bearer 123" }
* **body** String body, could be JSON. Example: "Hello, I am the body of the POST request."
* **fields** Same as above but table is automatically parsed to JSON. Example: { key = "123", name = "Joseph" }

Returns a table with body (string), status (int), headers (table) and elapsed (float). Nil if failed.
```lua
{
  status = 200,
  elapsed = 0.123,
  body = "content here",
  headers = {
    ["Host"] = "www.httpbin.org" 
  }
}
```

#### http_head (Sync)
Send a Head request.
```lua
http_head(url, headers)
http_head(url, headers, params)
```
* **url** The URL to send the request to. Example: https://google.com
* **headers** Table of headers to send. Example: { myHeader = "hi", anotherHeader = "lol", authentication = "bearer 123" }
* **params** Parameters to add to the request. Example: { apiKey = "123" }

Returns a table with body (string), status (int), headers (table) and elapsed (float). Nil if failed.
```lua
{
  status = 200,
  elapsed = 0.123,
  body = "content here",
  headers = {
    ["Host"] = "www.httpbin.org" 
  }
}
```

#### http_delete (Sync)
Send a Delete request.
```lua
http_delete(url, headers, body)
http_delete(url, headers, fields)
```
* **url** The URL to send the request to. Example: https://google.com
* **headers** Table of headers to send. Example: { myHeader = "hi", anotherHeader = "lol", authentication = "bearer 123" }
* **body** String body, could be JSON. Example: "Hello, I am the body of the POST request."
* **fields** Same as above but table is automatically parsed to JSON. Example: { key = "123", name = "Joseph" }

Returns a table with body (string), status (int), headers (table) and elapsed (float). Nil if failed.
```lua
{
  status = 200,
  elapsed = 0.123,
  body = "content here",
  headers = {
    ["Host"] = "www.httpbin.org" 
  }
}
```

#### http_patch (Sync)
Send a Patch request.
```lua
http_patch(url, headers, body)
http_patch(url, headers, fields)
```
* **url** The URL to send the request to. Example: https://google.com
* **headers** Table of headers to send. Example: { myHeader = "hi", anotherHeader = "lol", authentication = "bearer 123" }
* **body** String body, could be JSON. Example: "Hello, I am the body of the POST request."
* **fields** Same as above but table is automatically parsed to JSON. Example: { key = "123", name = "Joseph" }

Returns a table with body (string), status (int), headers (table) and elapsed (float). Nil if failed.
```lua
{
  status = 200,
  elapsed = 0.123,
  body = "content here",
  headers = {
    ["Host"] = "www.httpbin.org" 
  }
}
```
