import { useEffect, useMemo, useRef, useState } from "preact/hooks";
import type { PlayerColor } from "../types";
import type { BoardPiece } from "../chessFen";
import { parseFenBoard, squareNameFromIndex } from "../chessFen";

interface ChessBoardProps {
  fen: string;
  selectedSquare: string | null;
  perspective: PlayerColor;
  disabled?: boolean;
  lastMove?: string | null;
  legalTargets?: Array<{ square: string; capture: boolean }>;
  checkSquare?: string | null;
  onAnimationChange?: (active: boolean) => void;
  onSquareClick: (square: string, piece: BoardPiece | null) => void;
}

interface MoveAnimation {
  fromIndex: number;
  toIndex: number;
  piece: BoardPiece;
  running: boolean;
  key: number;
}

const pieceColumn: Record<BoardPiece["type"], number> = {
  pawn: 0,
  knight: 1,
  bishop: 2,
  rook: 3,
  queen: 4,
  king: 5,
};

export function ChessBoard(props: ChessBoardProps) {
  const board = parseFenBoard(props.fen);
  const displayIndices = useMemo(() => createDisplayIndices(props.perspective), [props.perspective]);
  const previousFenRef = useRef<string | null>(null);
  const previousMoveRef = useRef<string | null>(null);
  const timeoutRef = useRef<number | null>(null);
  const [animation, setAnimation] = useState<MoveAnimation | null>(null);

  useEffect(() => {
    const previousFen = previousFenRef.current;
    const previousMove = previousMoveRef.current;
    const parsedMove = props.lastMove ? parseUciMove(props.lastMove) : null;

    if (previousFen && parsedMove && props.lastMove !== previousMove) {
      const previousBoard = parseFenBoard(previousFen);
      const piece = previousBoard[parsedMove.fromIndex] ?? board[parsedMove.toIndex] ?? null;

      if (piece) {
        if (timeoutRef.current !== null) {
          window.clearTimeout(timeoutRef.current);
        }

        props.onAnimationChange?.(true);
        setAnimation({
          fromIndex: parsedMove.fromIndex,
          toIndex: parsedMove.toIndex,
          piece,
          running: false,
          key: Date.now(),
        });

        window.requestAnimationFrame(() => {
          setAnimation((current) => (current ? { ...current, running: true } : current));
        });

        timeoutRef.current = window.setTimeout(() => {
          setAnimation(null);
          props.onAnimationChange?.(false);
          timeoutRef.current = null;
        }, 190);
      }
    }

    previousFenRef.current = props.fen;
    previousMoveRef.current = props.lastMove ?? null;
  }, [props.fen, props.lastMove]);

  useEffect(() => {
    return () => {
      if (timeoutRef.current !== null) {
        window.clearTimeout(timeoutRef.current);
      }
      props.onAnimationChange?.(false);
    };
  }, []);

  return (
    <div class="chess-board" aria-label="Chess board">
      {displayIndices.map((index) => {
        const isAnimationTarget = animation?.toIndex === index;
        const piece = isAnimationTarget ? null : (board[index] ?? null);
        const file = index % 8;
        const rank = Math.floor(index / 8);
        const isLight = (file + rank) % 2 === 0;
        const square = squareNameFromIndex(index);
        const isSelected = props.selectedSquare === square;
        const parsedLastMove = props.lastMove ? parseUciMove(props.lastMove) : null;
        const isLastMove = parsedLastMove?.fromIndex === index || parsedLastMove?.toIndex === index;
        const hint = props.legalTargets?.find((target) => target.square === square);
        const isCheck = props.checkSquare === square;

        return (
          <button
            key={square}
            class={[
              "chess-square",
              isLight ? "is-light" : "is-dark",
              isSelected ? "is-selected" : "",
              isLastMove ? "is-last-move" : "",
              isCheck ? "is-check" : "",
            ].join(" ")}
            type="button"
            disabled={props.disabled}
            onClick={() => props.onSquareClick(square, piece)}
          >
            <span class="square-label">{square}</span>
            {hint && <span class={hint.capture ? "move-hint is-capture" : "move-hint"} aria-hidden="true" />}
            {piece && <PieceSprite piece={piece} />}
          </button>
        );
      })}
      {animation && <AnimatedPiece animation={animation} displayIndices={displayIndices} />}
    </div>
  );
}

function createDisplayIndices(perspective: PlayerColor): number[] {
  const indices = Array.from({ length: 64 }, (_, index) => index);

  if (perspective === "black") {
    return indices.reverse();
  }

  return indices;
}

function PieceSprite({ piece }: { piece: BoardPiece }) {
  const col = pieceColumn[piece.type];
  const row = piece.color === "white" ? 0 : 1;

  return (
    <span
      class="piece-sprite"
      style={{
        backgroundPosition: `${col * 20}% ${row * 100}%`,
      }}
      aria-label={`${piece.color} ${piece.type}`}
    />
  );
}

function AnimatedPiece({ animation, displayIndices }: { animation: MoveAnimation; displayIndices: number[] }) {
  const from = squarePercent(animation.fromIndex, displayIndices);
  const to = squarePercent(animation.toIndex, displayIndices);

  return (
    <span
      key={animation.key}
      class="animated-piece"
      style={{
        transform: `translate(${animation.running ? to.x : from.x}%, ${animation.running ? to.y : from.y}%)`,
      }}
    >
      <PieceSprite piece={animation.piece} />
    </span>
  );
}

function squarePercent(index: number, displayIndices: number[]): { x: number; y: number } {
  const displayIndex = Math.max(0, displayIndices.indexOf(index));
  const file = displayIndex % 8;
  const rank = Math.floor(displayIndex / 8);

  return {
    x: file * 100,
    y: rank * 100,
  };
}

function parseUciMove(uci: string): { fromIndex: number; toIndex: number } | null {
  if (!/^[a-h][1-8][a-h][1-8][qrbn]?$/.test(uci)) {
    return null;
  }

  return {
    fromIndex: squareToIndex(uci.slice(0, 2)),
    toIndex: squareToIndex(uci.slice(2, 4)),
  };
}

function squareToIndex(square: string): number {
  const file = square.charCodeAt(0) - "a".charCodeAt(0);
  const rank = Number(square[1]);

  return (8 - rank) * 8 + file;
}
