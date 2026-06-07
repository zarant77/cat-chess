export class ApiError extends Error {
  public readonly statusCode: number;
  public readonly code: string;

  public constructor(statusCode: number, code: string) {
    super(code);
    this.name = "ApiError";
    this.statusCode = statusCode;
    this.code = code;
  }
}

export function badRequest(code: string): ApiError {
  return new ApiError(400, code);
}

export function unauthorized(code = "unauthorized"): ApiError {
  return new ApiError(401, code);
}

export function forbidden(code: string): ApiError {
  return new ApiError(403, code);
}

export function notFound(code: string): ApiError {
  return new ApiError(404, code);
}

export function conflict(code: string): ApiError {
  return new ApiError(409, code);
}
