# Scanner Hardware Web API Specification (draft)

This document outlines a draft specification for AATEA Solutions to handle HTTP POST requests from scanner hardware in the museum.

## Endpoints

### 1. Register Endpoint

**Endpoint:** `/register`

**Description:** This endpoint is used to register a node in the server dynamically. This is particularly useful for quickly setting up or replacing nodes.

**HTTP Method:** POST

**Parameters:**

- `node` (required): The MAC address of the node.
- `identifier` (required): The logical identifier which links the node to content.

**Example Request:**

```
POST /register HTTP/1.1
Host: yourserver.com
Content-Type: application/x-www-form-urlencoded
Content-Length: 43

node=00:14:22:01:23:45&identifier=Exhibit_A
```

**Responses:**

- `200 OK`: Node registered successfully.
- `500 Internal Server Error`: Registration failed, but cause is unclear.

### 2. Scan Endpoint

**Endpoint:** `/scan`

**Description:** This endpoint logs when a user (tag) scans a location (node).

**HTTP Method:** POST

**Parameters:**

- `node` (required): The MAC address of the node.
- `tag` (required): The numerical tag ID.

**Example Request:**

```
POST /scan HTTP/1.1
Host: yourserver.com
Content-Type: application/x-www-form-urlencoded
Content-Length: 33

node=00:14:22:01:23:45&tag=123456
```

**Responses:**

- `200 OK`: Scan logged successfully.
- `500 Internal Server Error`: Scan log failed, but cause is unclear.

## User Access URL

**Description:** Users can access the website via a URL to see resources linked to their scan.

**URL Pattern:** `/<scans/trip/other>/<tag_id>`

**Example URL:**

```
https://example.com/trip/123456
```

## General Response Codes

- `200 OK`: The request was successful and the server processed it correctly.
- `500 Internal Server Error`: The request was unsuccessful.

## Notes

- All requests are sent via POST, as the microcontrollers are only capable of minimal processing.
- Responses should be kept minimal, ideally just indicating success or failure with the appropriate status code.
- The controllers are only capable of displaying a success, waiting, or a failure to the visitor with blinking lights.
