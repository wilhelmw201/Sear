# Sear: Burn image without wiping your disk

This small tool merges the partition tables of the source image and the target disk and copies the data onto the free space of target disk while preserving the data on it.
This is tool is still under construction and I do suggest that you have some data recovery tools at hand when you use it. 

Linux support is WIP as my ubuntu is refusing to boot at the moment.

# Requirements

- The source image and the target disk must use the [GUID Partition Table (GPT)](https://de.wikipedia.org/wiki/GUID_Partition_Table) format. 
- You must have enough empty space at the start of your disk to contain the image.

# Freeing up space at start of disk 

- Windows:
Consider using [DiskGenius](https://www.diskgenius.com/). [diskpart](https://learn.microsoft.com/en-us/windows-server/administration/windows-commands/diskpart) should also work but it is not very user friendly.
- Linux:
Use gparted (`apt install gparted` if you don't have it). [Here](https://gparted.org/display-doc.php%3Fname%3Dmoving-space-between-partitions) is a mini guide.

Either way, make sure that you have enough unallocated space at the leftmost of your partition.

# Converting [MBR](https://de.wikipedia.org/wiki/Master_Boot_Record) to [GPT](https://gparted.org/display-doc.php%3Fname%3Dmoving-space-between-partitions)

- Windows:
Use [MBR2GPT](https://learn.microsoft.com/en-us/windows/deployment/mbr-to-gpt) to convert your target disk first. This tool will not lead to data loss. However, you may want to [free up space](Freeing-up-space-at-start-of-disk) first, such that you have space for the image at the beginning **and at least 17KB of unallocated space at the end of disk**. This makes the conversion process easier.



