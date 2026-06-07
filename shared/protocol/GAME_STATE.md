# Cat Chess Game State

This document describes the game state format used by the Cat Chess server, web client, and Android client.

The server is authoritative.

Clients should display the state returned by the server and send player actions back as requests.

---

# Game Object

A game returned by the API has this shape:

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
  "updatedAt": 1790000010,
  "startedAt": 1790000005,
  "finishedAt": null
}
```

---

# Fields

## `id`

```ts
id: number;
```

Internal server game ID.

Used by API endpoints:

```text
GET  /games/:id
GET  /games/:id/moves
POST /games/:id/move
POST /games/:id/resign
```

---

## `inviteCode`

```ts
inviteCode: string | null;
```

Temporary invite code used to join a waiting game.

Example:

```text
MURKOT
```

The invite code is reserved while the game is waiting for Black.

After Black joins, the code is released back into the invite pool.

The game may still keep the old invite code for display/debug purposes, but clients must not treat it as a permanent game ID.

---

## `status`

```ts
status: "waiting_for_black" | "active" | "finished";
```

Current lifecycle state of the game.

### `waiting_for_black`

The game was created by White.

Black has not joined yet.

Moves are not allowed.

### `active`

Both players joined.

Moves are allowed.

### `finished`

The game is over.

Moves are not allowed.

---

## `result`

```ts
result: null | "white_won" | "black_won" | "draw";
```

Final result of the game.

While the game is not finished, `result` is `null`.

Examples:

```json
{
  "status": "active",
  "result": null
}
```

```json
{
  "status": "finished",
  "result": "black_won"
}
```

---

## `boardFen`

```ts
boardFen: string;
```

Board state in FEN format.

Current MVP returns the initial FEN and does not yet update it after moves.

Current initial value:

```text
rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
```

Later, when full chess rules are implemented, this field will be updated after every legal move.

Clients should treat `boardFen` as the board source of truth.

---

## `sideToMove`

```ts
sideToMove: "white" | "black";
```

Which side must move next.

Current MVP toggles this value after every accepted move.

Example:

```json
{
  "sideToMove": "white"
}
```

If `yourColor` equals `sideToMove`, it is this device's turn.

---

## `yourColor`

```ts
yourColor: "white" | "black" | null;
```

Color of the current device in this game.

For normal authenticated game responses this should be:

```text
white
```

or:

```text
black
```

`null` is reserved for future spectator/debug use.

---

## `createdAt`

```ts
createdAt: number;
```

Unix timestamp in seconds.

When the game was created.

---

## `updatedAt`

```ts
updatedAt: number;
```

Unix timestamp in seconds.

Updated when the game changes:

```text
Black joins
move is made
game is resigned
game is finished
```

---

## `startedAt`

```ts
startedAt: number | null;
```

Unix timestamp in seconds.

Set when Black joins and the game becomes active.

`null` while the game is still waiting for Black.

---

## `finishedAt`

```ts
finishedAt: number | null;
```

Unix timestamp in seconds.

Set when the game is finished.

`null` while the game is not finished.

---

# Move Object

A move returned by `GET /games/:id/moves` has this shape:

```json
{
  "id": 1,
  "gameId": 1,
  "moveIndex": 0,
  "color": "white",
  "uci": "e2e4",
  "fenAfter": "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
  "createdAt": 1790000010
}
```

## `id`

```ts
id: number;
```

Internal server move ID.

## `gameId`

```ts
gameId: number;
```

Game ID this move belongs to.

## `moveIndex`

```ts
moveIndex: number;
```

Zero-based move index inside the game.

First move:

```text
0
```

Second move:

```text
1
```

## `color`

```ts
color: "white" | "black";
```

Color that made the move.

## `uci`

```ts
uci: string;
```

Move in UCI-like format.

Examples:

```text
e2e4
g1f3
e7e8q
```

## `fenAfter`

```ts
fenAfter: string;
```

FEN after the move.

Current MVP stores the current game FEN without real board update.

Later this will contain the real updated FEN.

## `createdAt`

```ts
createdAt: number;
```

Unix timestamp in seconds.

When the move was accepted by the server.

---

# Client Rules

Clients should:

- store `deviceSecret` locally
- send it as `X-Cat-Chess-Device`
- load game state from `GET /games/:id`
- load move history from `GET /games/:id/moves`
- send moves using `POST /games/:id/move`
- never send a full board state as authority

Clients should not:

- trust local board state more than server state
- modify `sideToMove` locally as final truth
- treat `inviteCode` as permanent game ID
- allow moves when `status` is not `active`

---

# Display Rules

Basic UI logic:

```text
status = waiting_for_black
    Show invite code
    Show "Waiting for opponent"

status = active
    If yourColor == sideToMove:
        Show "Your move"
    Else:
        Show "Waiting for opponent"

status = finished
    Show result
```

Result display:

```text
white_won → White won
black_won → Black won
draw      → Draw
```

For the current device:

```text
result = white_won and yourColor = white → You won
result = black_won and yourColor = white → You lost
result = draw → Draw
```

---

# Current MVP Limitations

Current server MVP does not yet implement:

- full legal chess move validation
- real FEN updates after moves
- check/checkmate detection
- castling validation
- en passant validation
- promotion validation beyond basic UCI format
- draw offers
- timeout logic
- live presence
- push notifications
