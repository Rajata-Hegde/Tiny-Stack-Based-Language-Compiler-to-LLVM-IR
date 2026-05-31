// Statistics Calculator - Compute mean and variance of sample data
// Demonstrating complex float arithmetic

// Sample data: [10.5, 20.3, 15.2, 18.9, 12.1]

10.5 data1 =
20.3 data2 =
15.2 data3 =
18.9 data4 =
12.1 data5 =

// Calculate sum
data1 data2 + data3 + data4 + data5 + sum =

// Calculate mean: sum / 5
sum 5.0 / mean =

// Print mean
mean print

// Calculate squared deviations from mean
// (data1 - mean)^2
data1 mean - dup * dev1_sq =
dev1_sq print

// (data2 - mean)^2
data2 mean - dup * dev2_sq =
dev2_sq print

// (data3 - mean)^2
data3 mean - dup * dev3_sq =
dev3_sq print

// (data4 - mean)^2
data4 mean - dup * dev4_sq =
dev4_sq print

// (data5 - mean)^2
data5 mean - dup * dev5_sq =
dev5_sq print

// Sum of squared deviations
dev1_sq dev2_sq + dev3_sq + dev4_sq + dev5_sq + sum_sq =

// Variance: sum_sq / (n-1) = sum_sq / 4
sum_sq 4.0 / variance =

// Print variance
variance print
