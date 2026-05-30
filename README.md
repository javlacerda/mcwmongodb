# MCWMongoDB

**MCWMongoDB** is an **Omnis Studio external component (xcomp)** that provides integration with **MongoDB (NoSQL)** using the official **Mongo C Driver**.

It enables Omnis Studio applications to connect, query, and filter MongoDB collections with a simple API.


## Features

- Connect to MongoDB using a connection string
- Select database and collection
- Query documents with filters
- Support for dynamic MongoDB query expressions
- Typed filtering (string, integer, boolean, datetime, etc.)
- Error handling and diagnostics


## Requirements

- Omnis Studio
- Visual Studio 2022
- vcpkg
- mongo-c-driver:x64-windows

Install dependency:

```
vcpkg install mongo-c-driver:x64-windows
```


## Build Instructions

```
vcpkg integrate install
```

- Open solution in Visual Studio 2022
- Set platform to x64
- Build solution

