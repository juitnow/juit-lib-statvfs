'use strict'

const os = require('node:os')
const native = require(`./${os.platform()}-${os.arch()}/statvfs.node`)

module.exports = native
