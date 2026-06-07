# Cat Chess API Draft

## Create game

```http
POST /games
```

Request:

```json
{
  "code": "MURKOT"
}
```

Response:

```json
{
  "code": "MURKOT",
  "color": "white",
  "secret": "client-secret-token"
}
```

## Join game

```http
POST /games/MURKOT/join
```

Response:

```json
{
  "code": "MURKOT",
  "color": "black",
  "secret": "client-secret-token"
}
```

## Get game state

```http
GET /games/MURKOT
```

## Submit move

```http
POST /games/MURKOT/move
```

Request:

```json
{
  "secret": "client-secret-token",
  "move": "e2e4"
}
```
