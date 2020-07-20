'use strict'

/* eslint no-invalid-this: off */
const { expect } = require('chai')
const mockery = require('mockery')

describe('StatVFS interface', () => {
  let statvfs = null, args = [], error = null

  before(() => {
    mockery.registerMock('./lib/statvfs-darwin-x64.node', (path, callback) => {
      expect(path).to.eql('/the-path')
      if (error) throw error
      callback(...args)
    })

    mockery.enable({
      warnOnReplace: false,
      warnOnUnregistered: false,
    })
  })

  it('should load the module', () => {
    const index = require('..')
    expect(index).to.be.a('function')
    statvfs = index
  })

  it('should invoke successfully', async () => {
    if (!statvfs) return this.skip()

    args = [ undefined, 1, 2, 3 ], error = null
    expect(await statvfs('/the-path')).to.eql({
      total: 1,
      available: 2,
      free: 3,
    })
  })

  it('should reject without a path', function() {
    if (!statvfs) return this.skip()

    expect(statvfs()).to.be.rejectedWith(TypeError, 'No path specified')
    expect(statvfs(true)).to.be.rejectedWith(TypeError, 'Invalid type for path')
  })

  it('should reject fail on callback error', async () => {
    if (!statvfs) return this.skip()

    args = [ new TypeError('Foo Bar Baz') ], error = null
    expect(statvfs('/the-path')).to.be.rejectedWith(TypeError, 'Foo Bar Baz')
  })

  it('should reject when native throws', async () => {
    if (!statvfs) return this.skip()

    args = [], error = new TypeError('Foo Bar Baz')
    expect(statvfs('/the-path')).to.be.rejectedWith(TypeError, 'Foo Bar Baz')
  })
})
