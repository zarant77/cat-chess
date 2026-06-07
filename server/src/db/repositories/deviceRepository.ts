import { db } from "../database.js";

export interface DeviceRow {
  id: number;
  device_hash: string;
  created_at: number;
  last_seen_at: number;
}

export function upsertDevice(deviceHash: string, now: number): void {
  db.prepare(
    `
    INSERT INTO devices (
      device_hash,
      created_at,
      last_seen_at
    )
    VALUES (?, ?, ?)
    ON CONFLICT(device_hash) DO UPDATE SET
      last_seen_at = excluded.last_seen_at
  `,
  ).run(deviceHash, now, now);
}
