##  Progress meeting 18/06/18

Goals from last week:
- Use new data (2 backgrounds)
- Test more neural networks
- Test more BDTs
- Better ways to compare models

---

### New Data

- Can now use the new datasets, but have to remove nulls/infs and ignore constants
  - Dummy Variables
- Standard way to deal with 2 backgrounds in TMVA is to train 2 classifiers - done

---

### Neural Networks

Implemented a [self-normalizing neural network](https://arxiv.org/abs/1706.02515)
- Used SELU, AlphaDropout and LeCun uniform initializer
- Some improvement

### BDTs

Tested a more configs

Both methods benefit from more data, large improvement from 8K to 20K
- more data?

---

### Application

Looked into application using reader class that would be used on event by event basis

![method_comparison](https://github.com/mj-will/ml4np/blob/master/figures/comp/rejBvsS.png?raw=true)

---

![BDThist](https://github.com/mj-will/ml4np/blob/master/figures/histBDT.png?raw=true) ![PKhist](https://github.com/mj-will/ml4np/blob/master/figures/histPyKeras.png?raw=true)

---
