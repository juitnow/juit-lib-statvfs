import { log } from '@plugjs/plug'

import { statvfs } from '../src/index'

describe('StatVFS', () => {
  it('should run its native component', async () => {
    const stats = await statvfs('/')
    expect(Object.keys(stats)).toEqual(jasmine.arrayContaining([
      'blocks_total',
      'blocks_free',
      'blocks_available',
      'blocks_used',
      'bytes_total',
      'bytes_free',
      'bytes_available',
      'bytes_used',
      'files_total',
      'files_free',
      'files_used',
      'percentage_used',
      'percentage_free',
    ]))

    expect(typeof stats.blocks_total).toBe('bigint')
    expect(typeof stats.blocks_free).toBe('bigint')
    expect(typeof stats.blocks_available).toBe('bigint')
    expect(typeof stats.blocks_used).toBe('bigint')
    expect(typeof stats.bytes_total).toBe('bigint')
    expect(typeof stats.bytes_free).toBe('bigint')
    expect(typeof stats.bytes_available).toBe('bigint')
    expect(typeof stats.bytes_used).toBe('bigint')
    expect(typeof stats.files_total).toBe('bigint')
    expect(typeof stats.files_free).toBe('bigint')
    expect(typeof stats.files_used).toBe('bigint')
    expect(typeof stats.percentage_used).toBe('number')
    expect(typeof stats.percentage_free).toBe('number')

    expect(stats.blocks_total).toBeGreaterThan(0)
    expect(stats.blocks_free).toBeGreaterThan(0)
    expect(stats.blocks_available).toBeGreaterThan(0)
    expect(stats.blocks_used).toBeGreaterThan(0)
    expect(stats.bytes_total).toBeGreaterThan(0)
    expect(stats.bytes_free).toBeGreaterThan(0)
    expect(stats.bytes_available).toBeGreaterThan(0)
    expect(stats.bytes_used).toBeGreaterThan(0)
    expect(stats.files_total).toBeGreaterThan(0)
    expect(stats.files_free).toBeGreaterThan(0)
    expect(stats.files_used).toBeGreaterThan(0)
    expect(stats.percentage_used).toBeGreaterThan(0)
    expect(stats.percentage_free).toBeGreaterThan(0)

    log(stats)
  })
})
