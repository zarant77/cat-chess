import { createDevice } from "./api";

const DEVICE_SECRET_KEY = "cat_chess_device_secret";

export function getStoredDeviceSecret(): string | null {
  return localStorage.getItem(DEVICE_SECRET_KEY);
}

export function setStoredDeviceSecret(deviceSecret: string): void {
  localStorage.setItem(DEVICE_SECRET_KEY, deviceSecret);
}

export async function getOrCreateDeviceSecret(): Promise<string> {
  const stored = getStoredDeviceSecret();

  if (stored) {
    return stored;
  }

  const response = await createDevice();
  setStoredDeviceSecret(response.deviceSecret);

  return response.deviceSecret;
}
