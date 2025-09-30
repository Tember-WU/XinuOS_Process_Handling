# XinuOS Process Handling

Project #1 of **ECE 565: Operating Systems Design** at **NC State University**.

This project involved implementing process handling features in the Xinu operating system.  
Two coding problems were solved and passed all provided test cases.

---

## Features Implemented

### 1. Cascading Termination
- Group processes into **system processes** and **user processes**.
- Modify the `kill` function to perform **cascading termination** for **user processes only**.
- System processes remain unaffected.

### 2. Process Creation & Stack Handling
- Implement a `fork()` system call similar to **Unix `fork()`**.
- Ensures correct **process creation** and **stack handling** in the Xinu environment.

---

## Status
- Both problems implemented.
- All test cases passed successfully.

---

## Repository Structure
- `system/` → Modified kernel files  
- `fork.c` → Implementation of the `fork()` system call  
- `kill.c` → Modified process termination logic  

---
