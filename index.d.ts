/** Disk usage statistics from `statvfs`. */
export interface VFSStats {
  /** Size of the filesystem (in bytes). */
  total: number,
  /** Bytes available to unprivileged users. */
  available: number,
  /** Free bytes in the filesystem. */
  free: number,
}

type VFSCallback = (error: Error | null, stats: VFSStats) => void

/**
 * Return a `Promise` to a `VFSStats` structure describing total, available and
 * free bytes for the filesystem underlying the specified path.
 */
declare function statvfs(path: string): Promise<VFSStats>

/**
 * Get the total, available and free bytes for the filesystem underlying the
 * specified path and invoke a callback asynchronously.
 */
declare function statvfs(path: string, cb: VFSCallback): void

export default statvfs
