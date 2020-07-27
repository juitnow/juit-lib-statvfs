#!/usr/bin/env node

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

  const result = await(statvfs(path))
  for (const key in result) {
    const value = result[key]
    if (typeof value !== 'number') continue
    console.log(key.padStart(9), Math.round(value / divider))
  }
}

main(...process.argv.slice(2)).catch((error) => console.error(error))
