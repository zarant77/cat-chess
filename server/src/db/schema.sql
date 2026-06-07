PRAGMA journal_mode = WAL;
PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS devices (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    device_hash TEXT NOT NULL UNIQUE,
    created_at INTEGER NOT NULL,
    last_seen_at INTEGER NOT NULL
);

CREATE TABLE IF NOT EXISTS invite_codes (
    code TEXT PRIMARY KEY,
    status TEXT NOT NULL,
    reserved_game_id INTEGER,
    reserved_at INTEGER,
    used_count INTEGER NOT NULL DEFAULT 0
);

CREATE TABLE IF NOT EXISTS games (
    id INTEGER PRIMARY KEY AUTOINCREMENT,

    invite_code TEXT,
    status TEXT NOT NULL,

    board_fen TEXT NOT NULL,
    side_to_move TEXT NOT NULL,

    white_device_hash TEXT NOT NULL,
    black_device_hash TEXT,

    player_pair_key TEXT,

    created_at INTEGER NOT NULL,
    updated_at INTEGER NOT NULL,
    started_at INTEGER,
    finished_at INTEGER,

    FOREIGN KEY (white_device_hash) REFERENCES devices(device_hash),
    FOREIGN KEY (black_device_hash) REFERENCES devices(device_hash)
);

CREATE TABLE IF NOT EXISTS moves (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    game_id INTEGER NOT NULL,

    move_index INTEGER NOT NULL,
    color TEXT NOT NULL,
    uci TEXT NOT NULL,
    fen_after TEXT NOT NULL,

    created_at INTEGER NOT NULL,

    FOREIGN KEY (game_id) REFERENCES games(id)
);

CREATE UNIQUE INDEX IF NOT EXISTS idx_games_waiting_invite_code
ON games(invite_code)
WHERE status = 'waiting_for_black' AND invite_code IS NOT NULL;

CREATE UNIQUE INDEX IF NOT EXISTS idx_games_unfinished_pair
ON games(player_pair_key)
WHERE finished_at IS NULL AND player_pair_key IS NOT NULL;

CREATE INDEX IF NOT EXISTS idx_games_white_device
ON games(white_device_hash);

CREATE INDEX IF NOT EXISTS idx_games_black_device
ON games(black_device_hash);

CREATE INDEX IF NOT EXISTS idx_moves_game_id
ON moves(game_id);