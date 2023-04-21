import { log } from '@plugjs/plug'

import { statvfs } from '../src/index'

describe('StatVFS', () => {
  it('should run its native component', async () => {
    const stats = await statvfs('/')
    expect(stats).toEqual({
      blocks_total: expect.toBeA('bigint'),
      blocks_free: expect.toBeA('bigint'),
      blocks_available: expect.toBeA('bigint'),
      blocks_used: expect.toBeA('bigint'),
      bytes_total: expect.toBeA('bigint'),
      bytes_free: expect.toBeA('bigint'),
      bytes_available: expect.toBeA('bigint'),
      bytes_used: expect.toBeA('bigint'),
      files_total: expect.toBeA('bigint'),
      files_free: expect.toBeA('bigint'),
      files_used: expect.toBeA('bigint'),
      percentage_used: expect.toBeA('number'),
      percentage_free: expect.toBeA('number'),
    })

    expect(stats.blocks_total).toBeGreaterThan(0n)
    expect(stats.blocks_free).toBeGreaterThan(0n)
    expect(stats.blocks_available).toBeGreaterThan(0n)
    expect(stats.blocks_used).toBeGreaterThan(0n)
    expect(stats.bytes_total).toBeGreaterThan(0n)
    expect(stats.bytes_free).toBeGreaterThan(0n)
    expect(stats.bytes_available).toBeGreaterThan(0n)
    expect(stats.bytes_used).toBeGreaterThan(0n)
    expect(stats.files_total).toBeGreaterThan(0n)
    expect(stats.files_free).toBeGreaterThan(0n)
    expect(stats.files_used).toBeGreaterThan(0n)
    expect(stats.percentage_used).toBeGreaterThan(0)
    expect(stats.percentage_free).toBeGreaterThan(0)

    log(stats)
  })
})
