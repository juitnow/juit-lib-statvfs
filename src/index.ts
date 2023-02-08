/* eslint-disable camelcase */

import * as native from '../native/statvfs.cjs'

export interface FilesystemStats {
  /** The total number of blocks in the file system */
  blocks_total: bigint,
  /** The total number of **free** blocks in the file system */
  blocks_free: bigint,
  /** The total number of blocks _available to non-superuser_ in the file system */
  blocks_available: bigint,
  /** The total number of **used** blocks in the file system */
  blocks_used: bigint,

  /** The total number of bytes in the file system */
  bytes_total: bigint,
  /** The total number of **free** bytes in the file system */
  bytes_free: bigint,
  /** The total number of bytes _available to non-superuser_ in the file system */
  bytes_available: bigint,
  /** The total number of **used** bytes in the file system */
  bytes_used: bigint,

  /** The total number of files in the file system */
  files_total: bigint,
  /** The total number of **free** files in the file system */
  files_free: bigint,
  /** The total number of **used** files in the file system */
  files_used: bigint,

  /** The percentage of **used** blocks/bytes in the file system */
  percentage_used: number,
  /** The percentage of **free** blocks/bytes in the file system */
  percentage_free: number,
}

export function statvfs(path: string): Promise<FilesystemStats> {
  return new Promise((resolve, reject) => {
    native.statvfs(path, (error: Error | null, stats?: native.StatVFSData) => {
      if (error) return reject(error)

      if (stats) {
        const block_size = stats.f_frsize

        const blocks_total = stats.f_blocks
        const blocks_free = stats.f_bfree
        const blocks_available = stats.f_bavail
        const blocks_used = blocks_total - blocks_free

        const bytes_total = block_size * blocks_total
        const bytes_free = block_size * blocks_free
        const bytes_available = block_size * blocks_available
        const bytes_used = bytes_total - bytes_free

        const files_total = stats.f_files
        const files_free = stats.f_ffree
        const files_used = files_total - files_free

        const percentage_used = Number((blocks_used * 10000n) / blocks_total) / 100
        const percentage_free = Number((blocks_free * 10000n) / blocks_total) / 100

        return resolve({
          blocks_total, blocks_free, blocks_available, blocks_used,
          bytes_total, bytes_free, bytes_available, bytes_used,
          files_total, files_free, files_used,
          percentage_used, percentage_free,
        })
      }

      return reject(new Error('No error or stats received'))
    })
  })
}
