export function normalizeUciMove(uci: string): string {
  return uci.trim().toLowerCase();
}

export function isBasicUciMove(uci: string): boolean {
  return /^[a-h][1-8][a-h][1-8][qrbn]?$/.test(uci);
}
