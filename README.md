Disk Statistics for NodeJS
==========================

... or ???
> a wrapper adound `statvfs`

This library just wraps the `statvfs` system call, and returns all known
details to the caller in a nice `Promise`.

```typescript
import { statvfs } from '@juit/lib-statvfs'

// get the stats for our "/" (root) file system
const stats = await statvfs('/')

import { statvfs } from '../src/index'
// `stats` will now contain
// {
//   blocks_total: 1023659n,
//   blocks_free: 170084n,
//   blocks_available: 118826n,
//   blocks_used: 853575n,
//   bytes_total: 4192907264n,
//   bytes_free: 696664064n,
//   bytes_available: 486711296n,
//   bytes_used: 3496243200n,
//   files_total: 262144n,
//   files_free: 158621n,
//   files_used: 103523n,
//   percentage_used: 83.38,
//   percentage_free: 16.61
// }
```

What's being returned?
----------------------

#### Blocks!

* `blocks_total` _(type: `bigint`)_:
  The total number of blocks in the file system
* `blocks_free` _(type: `bigint`)_:
  The total number of **free** blocks in the file system
* `blocks_available` _(type: `bigint`)_:
  The total number of blocks _available to non-superuser_ in the file system
* `blocks_used` _(type: `bigint`)_:
  The total number of **used** blocks in the file system

#### Bytes!

* `bytes_total` _(type: `bigint`)_:
  The total number of bytes in the file system
* `bytes_free` _(type: `bigint`)_:
  The total number of **free** bytes in the file system
* `bytes_available` _(type: `bigint`)_:
  The total number of bytes _available to non-superuser_ in the file system
* `bytes_used` _(type: `bigint`)_:
  The total number of **used** bytes in the file system

#### Files!

* `files_total` _(type: `bigint`)_:
  The total number of files in the file system
* `files_free` _(type: `bigint`)_:
  The total number of **free** files in the file system
* `files_used` _(type: `bigint`)_:
  The total number of **used** files in the file system

#### ... and Percentages!

* `percentage_used` _(type: `number`)_:
  The percentage of **used** blocks/bytes in the file system
* `percentage_free` _(type: `number`)_:
  The percentage of **free** blocks/bytes in the file system
