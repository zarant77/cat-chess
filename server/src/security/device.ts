import { createHash, randomBytes } from "node:crypto";

const DEVICE_SECRET_BYTES = 32;

export function createDeviceSecret(): string {
  return randomBytes(DEVICE_SECRET_BYTES).toString("base64url");
}

export function hashDeviceSecret(deviceSecret: string): string {
  return createHash("sha256").update(deviceSecret).digest("hex");
}

export function isValidDeviceSecret(value: string): boolean {
  return /^[A-Za-z0-9_-]{43}$/.test(value);
}

export function createPlayerPairKey(a: string, b: string): string {
  return a < b ? `${a}:${b}` : `${b}:${a}`;
}
