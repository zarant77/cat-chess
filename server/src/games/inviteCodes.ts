import {
  findRandomFreeInviteCode,
  insertInviteCodeIfMissing,
  releaseInviteCodeByGameId,
  reserveInviteCode as reserveInviteCodeRow,
} from "../db/repositories/inviteCodeRepository.js";
import { DEFAULT_INVITE_CODES } from "./defaultInviteCodes.js";

export function seedInviteCodes(): void {
  for (const code of DEFAULT_INVITE_CODES) {
    insertInviteCodeIfMissing(code);
  }
}

export function reserveInviteCode(gameId: number, now: number): string {
  const row = findRandomFreeInviteCode();

  if (!row) {
    throw new Error("no_invite_codes_available");
  }

  const reserved = reserveInviteCodeRow(row.code, gameId, now);

  if (!reserved) {
    throw new Error("invite_code_race");
  }

  return row.code;
}

export function releaseInviteCodeForGame(gameId: number): void {
  releaseInviteCodeByGameId(gameId);
}
