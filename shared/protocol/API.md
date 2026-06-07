# Cat Chess API

Cat Chess server API for web and Android clients.

Base URL for local development:

```text
http://localhost:5400
```

## Authentication

Most game endpoints require a device secret.

Send it in the request header:

```http
X-Cat-Chess-Device: <device_secret>
```

The device secret is created by `POST /device`.

The client must store it locally and reuse it for future requests.

Do not send `deviceSecret` in query params.

---

# Health

## `GET /health`

Checks if the server and database are alive.

### Response

```json
{
  "ok": true,
  "service": "cat-chess-server"
}
```

---

# Device

## `POST /device`

Creates a new device secret or touches an existing one.

This endpoint does not require `X-Cat-Chess-Device`.

### Create new device

### Request

```json
{}
```

### Response

```json
{
  "deviceSecret": "random_device_secret"
}
```

### Touch existing device

### Request

```json
{
  "deviceSecret": "existing_device_secret"
}
```

### Response

```json
{
  "deviceSecret": "existing_device_secret"
}
```

---

# Games

## `POST /games`

Creates a new waiting game.

The creator becomes White.

The server reserves one invite code from the pool.

### Headers

```http
X-Cat-Chess-Device: <device_secret>
```

### Request

```json
{}
```

### Response

```json
{
  "id": 1,
  "inviteCode": "MURKOT",
  "status": "waiting_for_black",
  "result": null,
  "boardFen": "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKQBNR w KQkq - 0 1",
  "sideToMove": "white",
  "yourColor": "white",
  "createdAt": 1790000000,
  "updatedAt": 1790000000,
  "startedAt": null,
  "finishedAt": null
}
```

Note: actual `boardFen` is the standard initial FEN used by the server.

---

## `POST /games/join`

Joins a waiting game by invite code.

The joining player becomes Black.

After the game starts, the invite code is released back into the pool.

### Headers

```http
X-Cat-Chess-Device: <device_secret>
```

### Request

```json
{
  "inviteCode": "MURKOT"
}
```

### Response

```json
{
  "id": 1,
  "inviteCode": "MURKOT",
  "status": "active",
  "result": null,
  "boardFen": "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
  "sideToMove": "white",
  "yourColor": "black",
  "createdAt": 1790000000,
  "updatedAt": 1790000005,
  "startedAt": 1790000005,
  "finishedAt": null
}
```

---

## `GET /games`

Returns all games where the current device is White or Black.

### Headers

```http
X-Cat-Chess-Device: <device_secret>
```

### Response

```json
{
  "games": [
    {
      "id": 1,
      "inviteCode": "MURKOT",
      "status": "active",
      "result": null,
      "boardFen": "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
      "sideToMove": "white",
      "yourColor": "white",
      "createdAt": 1790000000,
      "updatedAt": 1790000005,
      "startedAt": 1790000005,
      "finishedAt": null
    }
  ]
}
```

---

## `GET /games/:id`

Returns one game if the current device is a participant.

### Headers

```http
X-Cat-Chess-Device: <device_secret>
```

### Response

```json
{
  "id": 1,
  "inviteCode": "MURKOT",
  "status": "active",
  "result": null,
  "boardFen": "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
  "sideToMove": "white",
  "yourColor": "white",
  "createdAt": 1790000000,
  "updatedAt": 1790000005,
  "startedAt": 1790000005,
  "finishedAt": null
}
```

---

## `GET /games/:id/moves`

Returns move history for a game.

Only participants can access it.

### Headers

```http
X-Cat-Chess-Device: <device_secret>
```

### Response

```json
{
  "moves": [
    {
      "id": 1,
      "gameId": 1,
      "moveIndex": 0,
      "color": "white",
      "uci": "e2e4",
      "fenAfter": "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
      "createdAt": 1790000010
    }
  ]
}
```

---

## `POST /games/:id/move`

Makes a move.

Current MVP only validates basic UCI format and turn order.

Full chess legality will be added later.

### Headers

```http
X-Cat-Chess-Device: <device_secret>
```

### Request

```json
{
  "uci": "e2e4"
}
```

Promotion example:

```json
{
  "uci": "e7e8q"
}
```

### Response

```json
{
  "id": 1,
  "inviteCode": "MURKOT",
  "status": "active",
  "result": null,
  "boardFen": "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
  "sideToMove": "black",
  "yourColor": "white",
  "createdAt": 1790000000,
  "updatedAt": 1790000010,
  "startedAt": 1790000005,
  "finishedAt": null
}
```

---

## `POST /games/:id/resign`

Resigns from an active game.

If White resigns, Black wins.

If Black resigns, White wins.

### Headers

```http
X-Cat-Chess-Device: <device_secret>
```

### Request

```json
{}
```

### Response

```json
{
  "id": 1,
  "inviteCode": "MURKOT",
  "status": "finished",
  "result": "black_won",
  "boardFen": "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
  "sideToMove": "white",
  "yourColor": "white",
  "createdAt": 1790000000,
  "updatedAt": 1790000100,
  "startedAt": 1790000005,
  "finishedAt": 1790000100
}
```

---

# Game Status

Possible `status` values:

```text
waiting_for_black
active
finished
```

## `waiting_for_black`

Game was created by White and is waiting for Black to join.

Invite code is reserved.

## `active`

Both players joined.

Moves are allowed.

Invite code is already released.

## `finished`

Game is over.

No moves are allowed.

---

# Game Result

Possible `result` values:

```text
null
white_won
black_won
draw
```

`result` is `null` while the game is not finished.

---

# Move Format

Moves use UCI-like format:

```text
e2e4
e7e8q
```

Basic format:

```text
[from file][from rank][to file][to rank][optional promotion]
```

Allowed promotion pieces:

```text
q
r
b
n
```

Examples:

```text
e2e4
g1f3
e7e8q
```

---

# Error Format

All API errors return:

```json
{
  "error": "error_code"
}
```

Validation errors return:

```json
{
  "error": "invalid_request",
  "issues": []
}
```

---

# Common Errors

## Authentication

```text
missing_device_secret
invalid_device_secret
```

## Games

```text
game_not_found
invite_code_not_found
cannot_join_own_game
game_between_players_already_exists
game_not_active
not_your_game
not_your_turn
```

## Moves

```text
invalid_move_format
```

---

# Notes

The server is authoritative.

Clients should not send a full board state.

Clients send player intent:

```json
{
  "uci": "e2e4"
}
```

The server validates the request, stores the move, updates the game state, and returns the updated game.

Current MVP does not yet implement full chess legality or real FEN updates after moves.
