/** The version of the native addon that was loaded */
export const version: string

/** Raw statistics about our filesystem */
export interface StatVFSData {
  /** Block size in bytes */
  f_frsize: bigint,
  /** Total data blocks in file system */
  f_blocks: bigint,
  /** Free blocks */
  f_bfree: bigint,
  /** Free blocks avail to non-superuser */
  f_bavail: bigint,
  /** Total file nodes in file system */
  f_files: bigint,
  /** Free file nodes */
  f_ffree: bigint,
}

/** Type for our {@link statvfs} callback */
type statvfs_callback =
  | ((error: Error, stats: undefined) => void)
  | ((error: null, stats: StatVFSData) => void)

/**
 * Return the statistics for a particular filesystem.
 *
 * @param path The path where the filesystem is mounted under
 * @param callback The callback to invoke after the native call is completed.
 */
export function statvfs(path: string, callback: statvfs_callback): void
