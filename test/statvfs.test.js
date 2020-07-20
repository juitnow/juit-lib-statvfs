'use strict'

/* eslint no-invalid-this: off */
const { expect } = require('chai')
const mockery = require('mockery')
const os = require('os')

describe('StatVFS interface', () => {
  describe('mocked native', () => {
    let statvfs = null, args = [], error = null

    before(() => {
      const binary = `./lib/statvfs-${os.type()}-${os.arch()}.node`.toLowerCase()
      mockery.registerMock(binary, (path, callback) => {
        expect(path).to.eql('/the-path')
        if (error) throw error
        setImmediate(() => callback(...args))
      })

      mockery.enable({
        warnOnReplace: false,
        warnOnUnregistered: false,
      })
    })

    after(() => {
      mockery.deregisterAll()
      mockery.disable()
    })

    it('should load the module', function() {
      this.slow(1000)

      const index = require('..')
      expect(index).to.be.a('function')
      statvfs = index
    })

    it('should invoke successfully', async function() {
      if (!statvfs) return this.skip()

      args = [ undefined, 1, 2, 3 ], error = null
      expect(await statvfs('/the-path')).to.eql({
        total: 1,
        available: 2,
        free: 3,
      })
    })

    it('should reject without a path', async function() {
      if (!statvfs) return this.skip()

      await expect(statvfs()).to.be.rejectedWith(TypeError, 'No path specified')
      await expect(statvfs(true)).to.be.rejectedWith(TypeError, 'Invalid type for path')
    })

    it('should reject fail on callback error', async function() {
      if (!statvfs) return this.skip()

      args = [ new TypeError('Foo Bar Baz') ], error = null
      await expect(statvfs('/the-path')).to.be.rejectedWith(TypeError, 'Foo Bar Baz')
    })

    it('should reject when native throws', async function() {
      if (!statvfs) return this.skip()

      args = [], error = new TypeError('Foo Bar Baz')
      await expect(statvfs('/the-path')).to.be.rejectedWith(TypeError, 'Foo Bar Baz')
    })
  })

  describe('real native', () => {
    before(() => {
      delete require.cache[require.resolve('..')]
    })

    it('should invoke successfully', async function() {
      this.slow(100)

      const statvfs = require('../index.js')
      const result = await statvfs('/')

      expect(result).to.be.an('object')
      expect(result).to.have.keys([ 'total', 'available', 'free' ])
      expect(result.total).to.be.a('number').greaterThan(100)
      expect(result.available).to.be.a('number').greaterThan(100)
      expect(result.free).to.be.a('number').greaterThan(100)
    })
  })
})
