# EE209-Customer-Manager
Design a customer manager using hash table in C language

**Implementation**
1.  Customer_manager_1 is a dynamically-resizable array
  - Starting size: 64
  - Inrease by 64 everytime running out of space
  
2. Customer_manager_2 is a dynamically-resizable hash table
  - Starting bucket size: 1024 entries
  - When the nodes reach 75% max capacity, double the number of bucket.
    For example, after the number of nodes reaches (0.75 * 1024), expend the hash table into 2048 bucket size
  - Check for potential reharsh of old data after table expension

**Features**
- Register customer with name, ID, purchase amount
- Unregister customer by name, ID
- Get purchase amount by name, ID
