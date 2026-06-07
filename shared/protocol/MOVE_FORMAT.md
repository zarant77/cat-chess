# Cat Chess Move Format

Cat Chess uses a simple UCI-like move format for client-server communication.

The client sends player intent.

The server validates and stores the move.

---

# Format

A move is encoded as:

```text
[from][to][promotion?]
```

Where:

```text
from       = source square
to         = target square
promotion  = optional promotion piece
```

Examples:

```text
e2e4
g1f3
e7e8q
```

---

# Square Format

A square is written as:

```text
[file][rank]
```

## File

```text
a b c d e f g h
```

## Rank

```text
1 2 3 4 5 6 7 8
```

Examples:

```text
a1
e4
h8
```

---

# Move Examples

## Normal move

```text
e2e4
```

Meaning:

```text
from e2
to   e4
```

## Knight move

```text
g1f3
```

Meaning:

```text
from g1
to   f3
```

## Capture

Captures are encoded the same way as normal moves.

```text
e4d5
```

The server determines whether the target square contains an enemy piece.

There is no `x` symbol in the move format.

## Promotion

Promotion adds one extra character at the end:

```text
e7e8q
```

Meaning:

```text
from      e7
to        e8
promotion queen
```

Allowed promotion pieces:

```text
q r b n
```

Where:

```text
q = queen
r = rook
b = bishop
n = knight
```

---

# Valid Move Regex

Current MVP basic validation:

```regex
^[a-h][1-8][a-h][1-8][qrbn]?$
```

Valid examples:

```text
e2e4
a7a8q
h2h1n
b1c3
```

Invalid examples:

```text
e2
e2-e4
E2E4
i2e4
e9e4
e2e4queen
cat
```

Note: clients may send uppercase or padded values, but the server normalizes moves before validation.

Server normalization:

```text
trim whitespace
convert to lowercase
```

So this:

```text
  E2E4
```

becomes:

```text
e2e4
```

---

# Coordinate Meaning

The board uses standard chess coordinates from White's perspective.

```text
White back rank: 1
Black back rank: 8
```

Initial position examples:

```text
White king: e1
Black king: e8
White pawns: rank 2
Black pawns: rank 7
```

---

# Client Input Flow

Recommended client flow:

```text
tap source square
tap target square
build UCI move
send POST /games/:id/move
reload game state from response
```

Example:

```json
{
  "uci": "e2e4"
}
```

Promotion flow:

```text
tap source pawn
tap promotion target square
show promotion picker
send move with promotion suffix
```

Example:

```json
{
  "uci": "e7e8q"
}
```

---

# Server Responsibilities

The server is responsible for:

```text
checking game exists
checking device is a participant
checking game is active
checking it is player's turn
validating move format
storing move
updating sideToMove
returning updated game
```

Later server versions will also be responsible for:

```text
checking legal chess movement
updating FEN
detecting check
detecting checkmate
detecting stalemate
handling castling
handling en passant
handling promotion
handling draw rules
```

---

# Current MVP Behavior

Current MVP validates only:

```text
basic UCI format
game status
participant access
turn order
```

Current MVP does not yet validate:

```text
piece exists on source square
piece belongs to current player
target square is legal
king safety
check
checkmate
castling rules
en passant
real promotion legality
```

Current MVP does not yet update `boardFen` after moves.

Moves are stored in history and `sideToMove` is toggled.

---

# API Endpoint

Moves are sent to:

```http
POST /games/:id/move
```

Headers:

```http
X-Cat-Chess-Device: <device_secret>
Content-Type: application/json
```

Body:

```json
{
  "uci": "e2e4"
}
```

Response:

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

# Why UCI-like Format

This format is:

```text
short
easy to type
easy to parse in C
easy to parse in TypeScript
compatible with chess tooling
good enough for network messages
```

It avoids verbose JSON like:

```json
{
  "from": "e2",
  "to": "e4",
  "promotion": null
}
```

Instead:

```json
{
  "uci": "e2e4"
}
```

Less noise. More chess.
