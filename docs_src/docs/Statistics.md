---
version: "3.0"
---
# Statistics
![3.0](https://img.shields.io/badge/Version-3.0-blue)

## Summary
This page describes the **Statistics** page: a chart of how a device has grown over time, for the device selected on the [Selection](Selection) page.

![Statistics page: chart of a storage device, with total file size, used space and total space](/img/statistics_01_overview.png)

## Where the data comes from {#data}
Each time a device is updated, and each time a **snapshot** is recorded (*Snapshot* on the [Devices](Devices#page-actions) page), Katalog keeps a record of its figures: number of files, total file size, and for storage devices, free and total space.<br/>
The chart draws these records. A device that was never updated, and never part of a snapshot, has no data: the chart shows *No data*.

## What is shown {#chart}
The chart title gives the device name, the data shown and its unit.

| Selected device | *Total File Size* | *Number of Files* |
|-----------------|-------------------|-------------------|
| Catalog | Total size of its files. | Number of its files. |
| Storage, Virtual | Total size of the files of its catalogs, plus the **used** and **total space** of the storage. | Number of files of its catalogs. |

Sizes are shown in the unit that fits the values (KiB, MiB, GiB or TiB).

## Options {#options}

| Option | Effect |
|--------|--------|
| *Source* | *all records*, *updates only* or *snapshots only*. Snapshots give regular points for the whole collection; updates add a point each time the device was updated. |
| *Data* | *Total File Size* or *Number of Files*. |
| *From date* | Shows only the records from this date. *Select a date* opens a dialog with quick choices (*1 day ago*, *1 week ago*, *1 month ago*, *1 year ago*); *Clear* shows all the records again. |
| *Display each value* | Marks each record with a point on the lines. |

The options are kept for the collection.

## Zoom {#zoom}
* **Click and drag** over the chart to zoom on that area.
* **Right-click** or **double-click** to come back to the full chart.

## Development
* See the backlog of [Statistics development](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=statistics).
