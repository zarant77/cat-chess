import Database from "better-sqlite3";
import { mkdirSync, readFileSync } from "node:fs";
import { dirname, join, resolve } from "node:path";
import { fileURLToPath } from "node:url";

const currentDir = dirname(fileURLToPath(import.meta.url));
const schemaPath = join(currentDir, "schema.sql");

const dataDir = resolve(process.cwd(), "data");
mkdirSync(dataDir, { recursive: true });

const databasePath = resolve(dataDir, "cat-chess.sqlite");

export const db = new Database(databasePath);

db.pragma("journal_mode = WAL");
db.pragma("foreign_keys = ON");

const schema = readFileSync(schemaPath, "utf8");
db.exec(schema);
