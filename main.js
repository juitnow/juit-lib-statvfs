'use strict'

const statvfs = require('.')

async function main(path, unit = 'b') {
  let divider = 1
  switch (unit.toLowerCase()) {
    case 'b': divider = 1; break
    case 'k': divider = 1024; break
    case 'm': divider = 1024 * 1024; break
    case 'g': divider = 1024 * 1024 * 1024; break
    default: throw new TypeError(`Invalid unit ${unit}`)
  }

  const results = await(statvfs(path))
  for (const key in results) {
    console.log(key.padStart(9), Math.round(results[key] / divider))
  }
}

main(...process.argv.slice(2)).catch((error) => console.error(error))
