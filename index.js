'use strict'

const os = require('os')
const native = require(`./lib/statvfs-${os.type()}-${os.arch()}.node`.toLowerCase())

module.exports = function statvfs(path) {
  const promise = new Promise((resolve, reject) => {
    if (! path) throw new TypeError('No path specified')
    if (typeof path !== 'string') throw new TypeError('Invalid type for path')

    console.log('FOO')
    native(path, (error, total, avail, free) => {
      console.log('BAR1', error, total, avail, free, promise)

      if (error) {
        reject(error)
      } else {
        resolve({
          total: total,
          available: avail,
          free: free,
        })
      }

      console.log('BAR2', error, total, avail, free, promise)

    })
  })

  return promise
}
