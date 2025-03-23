# Maximal Cliques Detection in Graphs

## Overview
This project focuses on detecting maximal cliques in graphs using three different algorithms:
- **Tomita et al.'s Algorithm**
- **Eppstein et al.'s Algorithm**
- **Chiba and Nishizeki's Algorithm**

The project evaluates these algorithms based on execution time across various datasets.

---
## Dataset Preparation
Before running the algorithms, the dataset must be preprocessed using `preprocess.cpp`.

### Preprocessing Steps:
1. Compile the preprocessing script:
   ```bash
   g++ -std=c++17 -O3 -o preprocess preprocess.cpp
   ```
2. Run the preprocessing script:
   ```bash
   ./preprocess <raw_input_file>.txt <processed_output_file>.txt
   ```
3. Use the `processed_output_file.txt` as input to the algorithm executables.

---


## Execution Instructions
Each of the three algorithms is implemented in a separate C++ file:
- `Tomita.cpp`
- `Eppstein.cpp`
- `Chiba.cpp`

To compile and execute each algorithm, use the following steps:

### Compilation:
```bash
# Compile Tomita Algorithm
g++ -std=c++17 -O3 -o tomita Tomita.cpp

# Compile Eppstein Algorithm
g++ -std=c++17 -O3 -o eppstein Eppstein.cpp

# Compile Chiba and Nishizeki Algorithm
g++ -std=c++17 -O3 -o chiba Chiba.cpp
```

### Execution:
```bash
# Run Tomita Algorithm
./tomita <input_file>.txt

# Run Eppstein Algorithm
./eppstein <input_file>.txt

# Run Chiba and Nishizeki Algorithm
./chiba <input_file>.txt
```

Replace `<input_file>.txt` with the appropriate dataset file.

---


## Individual Contributions
### Team Members and Contributions:
- **Deepanshu Garg (ID: 2021B3A72758H)** - Developed the project webpage and visual result presentations.
- **Shubham Birla (ID: 2021B3A72965H)** - Contributed to report writing, analysis, and graph visualizations.
- **Kinjal Vardia (ID: 2021B3A72579H)** - Implemented **Tomita et al.'s Algorithm** for maximal clique detection and conducted computational experiments.
- **Arihant Rai (ID: 2021B3A72507H)** - Implemented **Chiba and Nishizeki's Algorithm** focusing on arboricity and subgraph listing.
- **Mehul Kochar (ID: 2021B3A73032H)** - Implemented **Eppstein et al.'s Algorithm** for listing maximal cliques in near-optimal time.

---

## Project Webpage
For more details and visualizations, visit our project page:
👉 **[Project Webpage](https://yourprojectlink.com)** (Replace with actual GitHub/GitLab page link)

---

