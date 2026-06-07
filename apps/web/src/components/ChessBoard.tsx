import type { PlayerColor } from "../types";
import type { BoardPiece } from "../chessFen";
import { parseFenBoard, squareNameFromIndex } from "../chessFen";

interface ChessBoardProps {
  fen: string;
  selectedSquare: string | null;
  perspective: PlayerColor;
  onSquareClick: (square: string, piece: BoardPiece | null) => void;
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
  const displayIndices = createDisplayIndices(props.perspective);

  return (
    <div class="chess-board" aria-label="Chess board">
      {displayIndices.map((index) => {
        const piece = board[index] ?? null;
        const file = index % 8;
        const rank = Math.floor(index / 8);
        const isLight = (file + rank) % 2 === 0;
        const square = squareNameFromIndex(index);
        const isSelected = props.selectedSquare === square;

        return (
          <button
            key={square}
            class={["chess-square", isLight ? "is-light" : "is-dark", isSelected ? "is-selected" : ""].join(" ")}
            type="button"
            onClick={() => props.onSquareClick(square, piece)}
          >
            <span class="square-label">{square}</span>
            {piece && <PieceSprite piece={piece} />}
          </button>
        );
      })}
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
