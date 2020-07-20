'use strict'

const os = require('os')
const native = require(`./lib/statvfs-${os.type()}-${os.arch()}.node`.toLowerCase())

module.exports = function statvfs(path) {
  return new Promise((resolve, reject) => {
    if (! path) throw new TypeError('No path specified')
    if (typeof path !== 'string') throw new TypeError('Invalid type for path')

    native(path, (error, total, avail, free) => {
      if (error) return reject(error)
      resolve({
        total: total,
        available: avail,
        free: free,
      })
    })
  })
}
