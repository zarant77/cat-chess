import type { BoardPiece, BoardSquare, PieceType } from "./chessFen";
import { parseFenBoard, squareNameFromIndex } from "./chessFen";
import type { PlayerColor } from "./types";

export interface ParsedFen {
  board: BoardSquare[];
  sideToMove: PlayerColor;
  castling: string;
  enPassant: string | null;
  fullmove: number;
}

export interface LegalMove {
  from: string;
  to: string;
  promotion?: PromotionPiece;
  capture: boolean;
}

export type PromotionPiece = "queen" | "rook" | "bishop" | "knight";

export interface MoveLogEntry {
  moveNumber: number;
  color: PlayerColor;
  from: string;
  to: string;
  uci: string;
  display: string;
  captured: BoardPiece | null;
  promotion?: PromotionPiece;
}

const promotionPieces: PromotionPiece[] = ["queen", "rook", "bishop", "knight"];
const pieceValues: Record<PromotionPiece, string> = {
  queen: "q",
  rook: "r",
  bishop: "b",
  knight: "n",
};

export function parseFen(fen: string): ParsedFen {
  const parts = fen.trim().split(/\s+/);
  return {
    board: parseFenBoard(fen),
    sideToMove: parts[1] === "b" ? "black" : "white",
    castling: parts[2] && parts[2] !== "-" ? parts[2] : "",
    enPassant: parts[3] && parts[3] !== "-" ? parts[3] : null,
    fullmove: Number(parts[5]) || 1,
  };
}

export function squareToIndex(square: string): number {
  const file = square.charCodeAt(0) - "a".charCodeAt(0);
  const rank = Number(square[1]);
  return (8 - rank) * 8 + file;
}

export function indexToSquare(index: number): string {
  return squareNameFromIndex(index);
}

export function opposite(color: PlayerColor): PlayerColor {
  return color === "white" ? "black" : "white";
}

export function uciPromotionChar(piece?: PromotionPiece): string {
  return piece ? pieceValues[piece] : "";
}

export function isPromotionTarget(fen: string, from: string, to: string): boolean {
  const parsed = parseFen(fen);
  const piece = parsed.board[squareToIndex(from)];
  if (!piece || piece.type !== "pawn") {
    return false;
  }
  const rank = Number(to[1]);
  return rank === 1 || rank === 8;
}

export function legalMovesForSquare(fen: string, square: string): LegalMove[] {
  const parsed = parseFen(fen);
  const from = squareToIndex(square);
  const piece = parsed.board[from];
  if (!piece || piece.color !== parsed.sideToMove) {
    return [];
  }

  return generateLegalMoves(parsed).filter((move) => move.from === square);
}

export function generateLegalMoves(parsed: ParsedFen): LegalMove[] {
  const moves: LegalMove[] = [];
  for (let index = 0; index < 64; index += 1) {
    const piece = parsed.board[index];
    if (!piece || piece.color !== parsed.sideToMove) {
      continue;
    }
    for (const move of generatePseudoMoves(parsed, index, piece)) {
      const next = applyMove(parsed, move);
      if (!isKingInCheck(next.board, piece.color)) {
        moves.push(move);
      }
    }
  }
  return moves;
}

export function checkedKingSquare(fen: string): string | null {
  const parsed = parseFen(fen);
  return isKingInCheck(parsed.board, parsed.sideToMove) ? findKing(parsed.board, parsed.sideToMove) : null;
}

export function buildMoveLog(startFen: string, moves: { uci: string; color: PlayerColor }[]): MoveLogEntry[] {
  let parsed = parseFen(startFen);
  const entries: MoveLogEntry[] = [];

  for (const item of moves) {
    const move = moveFromUci(parsed, item.uci);
    if (!move) {
      continue;
    }
    const piece = parsed.board[squareToIndex(move.from)];
    if (!piece) {
      continue;
    }
    const next = applyMove(parsed, move);
    entries.push({
      moveNumber: parsed.fullmove,
      color: piece.color,
      from: move.from,
      to: move.to,
      uci: item.uci,
      display: formatMove(parsed, move, next),
      captured: capturedPiece(parsed, move),
      promotion: move.promotion,
    });
    parsed = {
      ...next,
      fullmove: piece.color === "black" ? parsed.fullmove + 1 : parsed.fullmove,
    };
  }

  return entries;
}

function generatePseudoMoves(parsed: ParsedFen, from: number, piece: BoardPiece): LegalMove[] {
  if (piece.type === "pawn") {
    return generatePawnMoves(parsed, from, piece);
  }
  if (piece.type === "knight") {
    return generateStepMoves(parsed, from, piece, [
      [1, 2],
      [2, 1],
      [2, -1],
      [1, -2],
      [-1, -2],
      [-2, -1],
      [-2, 1],
      [-1, 2],
    ]);
  }
  if (piece.type === "bishop") {
    return generateSlideMoves(parsed, from, piece, [
      [1, 1],
      [1, -1],
      [-1, 1],
      [-1, -1],
    ]);
  }
  if (piece.type === "rook") {
    return generateSlideMoves(parsed, from, piece, [
      [1, 0],
      [-1, 0],
      [0, 1],
      [0, -1],
    ]);
  }
  if (piece.type === "queen") {
    return generateSlideMoves(parsed, from, piece, [
      [1, 0],
      [-1, 0],
      [0, 1],
      [0, -1],
      [1, 1],
      [1, -1],
      [-1, 1],
      [-1, -1],
    ]);
  }
  return generateKingMoves(parsed, from, piece);
}

function generatePawnMoves(parsed: ParsedFen, from: number, piece: BoardPiece): LegalMove[] {
  const moves: LegalMove[] = [];
  const direction = piece.color === "white" ? -1 : 1;
  const startRank = piece.color === "white" ? 6 : 1;
  const { file, rank } = indexParts(from);
  const one = indexFromParts(file, rank + direction);
  if (one !== null && !parsed.board[one]) {
    addPawnMove(moves, from, one, false, piece.color);
    const two = indexFromParts(file, rank + direction * 2);
    if (rank === startRank && two !== null && !parsed.board[two]) {
      moves.push(moveOf(from, two, false));
    }
  }

  for (const delta of [-1, 1]) {
    const to = indexFromParts(file + delta, rank + direction);
    if (to === null) {
      continue;
    }
    const target = parsed.board[to];
    const toSquare = indexToSquare(to);
    if (target && target.color !== piece.color && target.type !== "king") {
      addPawnMove(moves, from, to, true, piece.color);
    } else if (parsed.enPassant === toSquare) {
      moves.push(moveOf(from, to, true));
    }
  }
  return moves;
}

function addPawnMove(moves: LegalMove[], from: number, to: number, capture: boolean, color: PlayerColor): void {
  const promotionRank = color === "white" ? 0 : 7;
  if (indexParts(to).rank === promotionRank) {
    promotionPieces.forEach((promotion) => moves.push(moveOf(from, to, capture, promotion)));
  } else {
    moves.push(moveOf(from, to, capture));
  }
}

function generateStepMoves(parsed: ParsedFen, from: number, piece: BoardPiece, offsets: number[][]): LegalMove[] {
  const moves: LegalMove[] = [];
  const { file, rank } = indexParts(from);
  for (const [df, dr] of offsets) {
    const to = indexFromParts(file + df, rank + dr);
    if (to === null) {
      continue;
    }
    const target = parsed.board[to];
    if (!target || (target.color !== piece.color && target.type !== "king")) {
      moves.push(moveOf(from, to, !!target));
    }
  }
  return moves;
}

function generateSlideMoves(parsed: ParsedFen, from: number, piece: BoardPiece, directions: number[][]): LegalMove[] {
  const moves: LegalMove[] = [];
  const start = indexParts(from);
  for (const [df, dr] of directions) {
    let file = start.file + df;
    let rank = start.rank + dr;
    while (true) {
      const to = indexFromParts(file, rank);
      if (to === null) {
        break;
      }
      const target = parsed.board[to];
      if (!target) {
        moves.push(moveOf(from, to, false));
      } else {
        if (target.color !== piece.color && target.type !== "king") {
          moves.push(moveOf(from, to, true));
        }
        break;
      }
      file += df;
      rank += dr;
    }
  }
  return moves;
}

function generateKingMoves(parsed: ParsedFen, from: number, piece: BoardPiece): LegalMove[] {
  const moves = generateStepMoves(parsed, from, piece, [
    [1, 0],
    [1, 1],
    [0, 1],
    [-1, 1],
    [-1, 0],
    [-1, -1],
    [0, -1],
    [1, -1],
  ]);

  const rank = piece.color === "white" ? 7 : 0;
  const enemy = opposite(piece.color);
  if (!isKingInCheck(parsed.board, piece.color)) {
    if (
      parsed.castling.includes(piece.color === "white" ? "K" : "k") &&
      !parsed.board[indexFromParts(5, rank) as number] &&
      !parsed.board[indexFromParts(6, rank) as number] &&
      !isSquareAttacked(parsed.board, indexFromParts(5, rank) as number, enemy) &&
      !isSquareAttacked(parsed.board, indexFromParts(6, rank) as number, enemy)
    ) {
      moves.push(moveOf(from, indexFromParts(6, rank) as number, false));
    }
    if (
      parsed.castling.includes(piece.color === "white" ? "Q" : "q") &&
      !parsed.board[indexFromParts(1, rank) as number] &&
      !parsed.board[indexFromParts(2, rank) as number] &&
      !parsed.board[indexFromParts(3, rank) as number] &&
      !isSquareAttacked(parsed.board, indexFromParts(2, rank) as number, enemy) &&
      !isSquareAttacked(parsed.board, indexFromParts(3, rank) as number, enemy)
    ) {
      moves.push(moveOf(from, indexFromParts(2, rank) as number, false));
    }
  }

  return moves;
}

function applyMove(parsed: ParsedFen, move: LegalMove): ParsedFen {
  const board = parsed.board.slice();
  const from = squareToIndex(move.from);
  const to = squareToIndex(move.to);
  const piece = board[from];
  if (!piece) {
    return { ...parsed, board };
  }

  const target = board[to];
  if (piece.type === "pawn" && parsed.enPassant === move.to && !target && from % 8 !== to % 8) {
    const capturedIndex = indexFromParts(to % 8, Math.floor(from / 8));
    if (capturedIndex !== null) {
      board[capturedIndex] = null;
    }
  }

  board[from] = null;
  board[to] = move.promotion ? { ...piece, type: move.promotion } : piece;

  if (piece.type === "king" && Math.abs((to % 8) - (from % 8)) === 2) {
    const rank = Math.floor(from / 8);
    if (to % 8 === 6) {
      const rookFrom = indexFromParts(7, rank) as number;
      const rookTo = indexFromParts(5, rank) as number;
      board[rookTo] = board[rookFrom];
      board[rookFrom] = null;
    } else {
      const rookFrom = indexFromParts(0, rank) as number;
      const rookTo = indexFromParts(3, rank) as number;
      board[rookTo] = board[rookFrom];
      board[rookFrom] = null;
    }
  }

  let castling = parsed.castling;
  if (piece.type === "king") {
    castling = removeCastlingRights(castling, piece.color === "white" ? ["K", "Q"] : ["k", "q"]);
  }
  if (piece.type === "rook") {
    castling = removeRookCastlingRight(castling, move.from);
  }
  if (target?.type === "rook") {
    castling = removeRookCastlingRight(castling, move.to);
  }

  const enPassant = piece.type === "pawn" && Math.abs(to - from) === 16 ? indexToSquare((to + from) / 2) : null;
  return { ...parsed, board, sideToMove: opposite(parsed.sideToMove), castling, enPassant };
}

function removeCastlingRights(castling: string, rights: string[]): string {
  return castling
    .split("")
    .filter((right) => !rights.includes(right))
    .join("");
}

function removeRookCastlingRight(castling: string, square: string): string {
  if (square === "h1") {
    return removeCastlingRights(castling, ["K"]);
  }
  if (square === "a1") {
    return removeCastlingRights(castling, ["Q"]);
  }
  if (square === "h8") {
    return removeCastlingRights(castling, ["k"]);
  }
  if (square === "a8") {
    return removeCastlingRights(castling, ["q"]);
  }
  return castling;
}

function capturedPiece(parsed: ParsedFen, move: LegalMove): BoardPiece | null {
  const from = squareToIndex(move.from);
  const to = squareToIndex(move.to);
  const piece = parsed.board[from];
  if (!piece) {
    return null;
  }
  const direct = parsed.board[to];
  if (direct) {
    return direct;
  }
  if (piece.type === "pawn" && parsed.enPassant === move.to && from % 8 !== to % 8) {
    const capturedIndex = indexFromParts(to % 8, Math.floor(from / 8));
    return capturedIndex === null ? null : parsed.board[capturedIndex];
  }
  return null;
}

function isKingInCheck(board: BoardSquare[], color: PlayerColor): boolean {
  const king = findKing(board, color);
  return king ? isSquareAttacked(board, squareToIndex(king), opposite(color)) : false;
}

function findKing(board: BoardSquare[], color: PlayerColor): string | null {
  const index = board.findIndex((piece) => piece?.type === "king" && piece.color === color);
  return index < 0 ? null : indexToSquare(index);
}

function isSquareAttacked(board: BoardSquare[], square: number, byColor: PlayerColor): boolean {
  const target = indexParts(square);
  for (let index = 0; index < 64; index += 1) {
    const piece = board[index];
    if (!piece || piece.color !== byColor) {
      continue;
    }
    const from = indexParts(index);
    const df = target.file - from.file;
    const dr = target.rank - from.rank;
    if (piece.type === "pawn") {
      const direction = byColor === "white" ? -1 : 1;
      if (dr === direction && Math.abs(df) === 1) {
        return true;
      }
    } else if (piece.type === "knight" && ((Math.abs(df) === 1 && Math.abs(dr) === 2) || (Math.abs(df) === 2 && Math.abs(dr) === 1))) {
      return true;
    } else if (piece.type === "king" && Math.max(Math.abs(df), Math.abs(dr)) === 1) {
      return true;
    } else if ((piece.type === "bishop" || piece.type === "queen") && Math.abs(df) === Math.abs(dr) && pathClear(board, from, target)) {
      return true;
    } else if ((piece.type === "rook" || piece.type === "queen") && (df === 0 || dr === 0) && pathClear(board, from, target)) {
      return true;
    }
  }
  return false;
}

function pathClear(board: BoardSquare[], from: { file: number; rank: number }, to: { file: number; rank: number }): boolean {
  const df = Math.sign(to.file - from.file);
  const dr = Math.sign(to.rank - from.rank);
  let file = from.file + df;
  let rank = from.rank + dr;
  while (file !== to.file || rank !== to.rank) {
    const index = indexFromParts(file, rank);
    if (index === null || board[index]) {
      return false;
    }
    file += df;
    rank += dr;
  }
  return true;
}

function moveOf(from: number, to: number, capture: boolean, promotion?: PromotionPiece): LegalMove {
  return { from: indexToSquare(from), to: indexToSquare(to), capture, promotion };
}

function moveFromUci(parsed: ParsedFen, uci: string): LegalMove | null {
  if (!/^[a-h][1-8][a-h][1-8][qrbn]?$/.test(uci)) {
    return null;
  }
  const from = uci.slice(0, 2);
  const to = uci.slice(2, 4);
  const promotion = promotionPieces.find((piece) => pieceValues[piece] === uci[4]);
  const legal = generateLegalMoves(parsed).find((move) => move.from === from && move.to === to && (!move.promotion || move.promotion === promotion));
  return legal ?? { from, to, promotion, capture: !!capturedPiece(parsed, { from, to, promotion, capture: false }) };
}

function formatMove(parsed: ParsedFen, move: LegalMove, after: ParsedFen): string {
  const from = squareToIndex(move.from);
  const to = squareToIndex(move.to);
  const piece = parsed.board[from];
  const suffix = isKingInCheck(after.board, after.sideToMove) ? (generateLegalMoves(after).length === 0 ? "#" : "+") : "";
  if (piece?.type === "king" && Math.abs((to % 8) - (from % 8)) === 2) {
    return `${to % 8 > from % 8 ? "O-O" : "O-O-O"}${suffix}`;
  }
  return `${move.from}${move.capture ? "x" : "-"}${move.to}${move.promotion ? `=${pieceLetter(move.promotion)}` : ""}${suffix}`;
}

function pieceLetter(piece: PieceType): string {
  return piece === "knight" ? "N" : piece[0].toUpperCase();
}

function indexParts(index: number): { file: number; rank: number } {
  return { file: index % 8, rank: Math.floor(index / 8) };
}

function indexFromParts(file: number, rank: number): number | null {
  if (file < 0 || file > 7 || rank < 0 || rank > 7) {
    return null;
  }
  return rank * 8 + file;
}
