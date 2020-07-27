'use strict'

const os = require('os')
const platform = `${os.type()}-${os.arch()}`.toLowerCase()
const native = require(`./native/${platform}/statvfs.node`)
const version = require('./package.json').version

/* istanbul ignore if */
if (native.version != version) {
  console.error(`WARNING: statvfs version mismatch: js=${version} native=${native.version}`)
}

module.exports = function statvfs(path) {
  return new Promise((resolve, reject) => {
    if (! path) throw new TypeError('No path specified')
    if (typeof path !== 'string') throw new TypeError('Invalid type for path')

    native.statvfs(path, (error, stat) => {
      if (error) return reject(error)
      const frsize = stat.f_frsize

      resolve({
        total: stat.f_blocks * frsize,
        available: stat.f_bavail * frsize,
        free: stat.f_bfree * frsize,
      })
    })
  })
}
