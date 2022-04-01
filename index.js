'use strict'

const os = require('os')
const platform = `${os.type()}-${os.arch()}`.toLowerCase()
const native = require(`./native/${platform}/statvfs.node`)

module.exports = function statvfs(path, cb) {
  if (! cb) {
    return new Promise((resolve, reject) => {
      try {
        statvfs(path, (error, result) => {
          if (error) reject(error)
          else resolve(result)
        })
      } catch (error) {
        reject(error)
      }
    })
  }

  if (! path) throw new TypeError('No path specified')
  if (typeof path !== 'string') throw new TypeError('Invalid type for path')

  native.statvfs(path, (error, stat) => {
    if (error) return cb(error)
    const frsize = stat.f_frsize

    cb(undefined, {
      total: stat.f_blocks * frsize,
      available: stat.f_bavail * frsize,
      free: stat.f_bfree * frsize,
    })
  })
}
