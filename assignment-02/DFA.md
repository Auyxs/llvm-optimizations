### Very Busy Expression Analysis

<img src="images/vbusy_exp.png" style="width: 50%">

### **VBE Framework**

| **Component**                | VBE                                                                 |
|------------------------------|---------------------------------------------------------------------|
| **Domain**                   | Sets of expressions                                                |
| **Direction**                | Backwards<br>In[b] = $f_b(\text{Out}[b])$<br>Out[b] = $\wedge \text{In}(\text{Succ}[b])$ |
| **Transfer Function**        | $f_b(x) = \text{Gen}[b] \cup (\text{In}[b] - \text{Kill}[b])$       |
| **Meet Operation** ($\wedge$)| $\cap$                                                             |
| **Boundary Condition**       | In[entry] = $\emptyset$                                           |
| **Initial Interior Points**  | In[b] = $\mathcal{U}$                                              |

- **Gen[b]**: The set of expressions **evaluated** within basic block *b*. These are the expressions that are guaranteed to be computed before any possible exit from the block.

- **Kill[b]**: The set of expressions that can no longer be considered "very busy" when exiting *b* because at least one of the variables used in them is **redefined** within the block. In other words, an expression is killed if one of its operands is modified inside *b*.



### **VBE Table**

| **Basic Block**   | **In[b]**         | **Out[b]**        |
|-------------------|-------------------|-------------------|
| **BB1 (entry)**   | $\{b-a\}$         | $\{b-a\}$         |
| **BB2**           | $\{b-a\}$         | $\{b-a\}$         |
| **BB3**           | $\{b-a, a-b\}$    | $\{a-b\}$         |
| **BB4**           | $\{a-b\}$         | $\emptyset$       |
| **BB5**           | $\{b-a\}$         | $\emptyset$       |
| **BB6**           | $\emptyset$       | $\{a-b\}$         |
| **BB7**           | $\{a-b\}$         | $\emptyset$       |
| **BB8 (exit)**    | $\emptyset$       | $\emptyset$       |

---

### Dominator Analysis

<img src="images/dom_analysis.png" style="width: 50%">

### **DA Framework**

| **Component**                | DA                                                                   |
|------------------------------|----------------------------------------------------------------------|
| **Domain**                   | Sets of Basic Blocks                                                 |
| **Direction**                | Forward<br>Out[b] = $f_b(\text{In}[b])$<br>In[b] = $\wedge \text{Out}(\text{Pred}[b])$ |
| **Transfer Function**        | $f_b(x) =  x \cup \{b\}$                                             |
| **Meet Operation** ($\wedge$)| $\cap$                                                              |
| **Boundary Condition**       | Out[entry] = $\emptyset$                                            |
| **Initial Interior Points**  | Out[b] = $\mathcal{U}$                                               |

### **DA Table**

| **Basic Block**   | **In[b]**       | **Out[b]**        |
|-------------------|-----------------|-------------------|
| **A (entry)**     | $\emptyset$     | \{A\}             |
| **B**             | \{A\}           | \{A, B\}          |
| **C**             | \{A\}           | \{A, C\}          |
| **D**             | \{A, C\}        | \{A, C, D\}       |
| **E**             | \{A, C\}        | \{A, C, E\}       |
| **F**             | \{A, C\}        | \{A, C, F\}       |
| **G (exit)**      | \{A\}           | \{A, G\}          |

---

### Constant Propagation

<img src="images/const_prop.png" style="width: 40%">


### **CP Framework**

| **Component**                | CP                                                                   |
|------------------------------|----------------------------------------------------------------------|
| **Domain**                   | Sets of pairs $(var, val)$                                           |
| **Direction**                | Forward<br>Out[b] = $f_b(\text{In}[b])$<br>In[b] = $\wedge \text{Out}(\text{Pred}[b])$ |
| **Transfer Function**        | $f_b(x) =  \text{Gen}[b] \cup (\text{In}[b] - \text{Kill}[b])$        |
| **Meet Operation** ($\wedge$)| $\cap$                                                              |
| **Boundary Condition**       | Out[entry] = $\emptyset$                                            |
| **Initial Interior Points**  | Out[b] = $\mathcal{U}$                                               |

- **Gen[b]**: The set of pairs \((\text{var}, \text{v})\) such that block *b* assigns a constant value to variable `var` (i.e., `var = const`). This also includes assignments like `var = expr` if the expression `expr` can be fully evaluated to a known constant using the current known constant values of its operands.


- **Kill[b]**: The set of pairs \((\text{var}, v)\) for all possible values `v`, where variable `var` is assigned a new value in block *b*, potentially overriding any previous constant value. This essentially kills any previous constant knowledge about `var`.

### **CP Table – First Iteration**

| **Basic Block**   | **In[b]**                                  | **Out[b]**                                         |
|-------------------|--------------------------------------------|----------------------------------------------------|
| **BB1 (entry)**   | $\emptyset$                                | $\emptyset$                                        |
| **BB2**           | $\emptyset$                                | $\{(k, 2)\}$                                       |
| **BB3**           | $\{(k, 2)\}$                              | $\{(k, 2)\}$                                       |
| **BB4**           | $\{(k, 2)\}$                              | $\{(k, 2), (a, 4)\}$                              |
| **BB5**           | $\{(k, 2), (a, 4)\}$                      | $\{(k, 2), (a, 4), (x, 5)\}$                      |
| **BB6**           | $\{(k, 2)\}$                              | $\{(k, 2), (a, 4)\}$                              |
| **BB7**           | $\{(k, 2), (a, 4)\}$                      | $\{(k, 2), (a, 4), (x, 8)\}$                      |
| **BB8**           | $\{(k, 2), (a, 4)\}$                      | $\{(k, 4), (a, 4)\}$                              |
| **BB9**           | $\{(k, 4), (a, 4)\}$                      | $\{(k, 4), (a, 4)\}$                              |
| **BB10**          | $\{(k, 4), (a, 4)\}$                      | $\{(k, 4), (a, 4), (b, 2)\}$                      |
| **BB11**          | $\{(k, 4), (a, 4), (b, 2)\}$              | $\{(k, 4), (a, 4), (b, 2), (x, 8)\}$              |
| **BB12**          | $\{(k, 4), (a, 4), (b, 2), (x, 8)\}$      | $\{(k, 4), (a, 4), (b, 2), (x, 8), (y, 8)\}$      |
| **BB13**          | $\{(k, 4), (a, 4), (b, 2), (x, 8), (y, 8)\}$ | $\{(k, 5), (a, 4), (b, 2), (x, 8), (y, 8)\}$      |
| **BB14**          | $\{(k, 5), (a, 4), (b, 2), (x, 8), (y, 8)\}$ | $\{(k, 5), (a, 4), (b, 2), (x, 8), (y, 8)\}$      |
| **BB15 (exit)**   | $\{(k, 5), (a, 4), (b, 2), (x, 8), (y, 8)\}$ | $\{(k, 5), (a, 4), (b, 2), (x, 8), (y, 8)\}$      |

### **CP Table – Second Iteration**

| **Basic Block**   | **In[b]**                                  | **Out[b]**                                      |
|-------------------|--------------------------------------------|-------------------------------------------------|
| **BB1 (entry)**   | $\emptyset$                                | $\emptyset$                                     |
| **BB2**           | $\emptyset$                                | $\{(k, 2)\}$                                    |
| **BB3**           | $\{(k, 2)\}$                              | $\{(k, 2)\}$                                    |
| **BB4**           | $\{(k, 2)\}$                              | $\{(k, 2), (a, 4)\}$                           |
| **BB5**           | $\{(k, 2), (a, 4)\}$                      | $\{(k, 2), (a, 4), (x, 5)\}$                    |
| **BB6**           | $\{(k, 2)\}$                              | $\{(k, 2), (a, 4)\}$                           |
| **BB7**           | $\{(k, 2), (a, 4)\}$                      | $\{(k, 2), (a, 4), (x, 8)\}$                    |
| **BB8**           | $\{(k, 2), (a, 4)\}$                      | $\{(k, 4), (a, 4)\}$                           |
| **BB9**           | $\{(a, 4)\}$                              | $\{(a, 4)\}$                                    |
| **BB10**          | $\{(a, 4)\}$                              | $\{(a, 4), (b, 2)\}$                           |
| **BB11**          | $\{(a, 4), (b, 2)\}$                      | $\{(a, 4), (b, 2)\}$                           |
| **BB12**          | $\{(a, 4), (b, 2)\}$                      | $\{(k, 4), (a, 4), (b, 2), (y, 8)\}$           |
| **BB13**          | $\{(a, 4), (b, 2), (y, 8)\}$              | $\{(a, 4), (b, 2), (y, 8)\}$                   |
| **BB14**          | $\{(a, 4), (b, 2), (y, 8)\}$              | $\{(a, 4), (b, 2), (y, 8)\}$                   |
| **BB15 (exit)**   | $\{(a, 4), (b, 2), (y, 8)\}$              | $\{(a, 4), (b, 2), (y, 8)\}$                   |


### **Convergence**

Since there are no further changes to the `Out[b]` sets in the second iteration, the constant propagation analysis converges.