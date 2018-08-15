##  Final Presentation

---

### Multivariate analysis

The work builds on ROOT TMVA and ties in with the HASPECT6 package.

Makes MVA easier to run:

* Specify the particles in the reaction
* Specfiy the methods to be trained
* Train
* Apply

---

### Variables

The MVA acts as another 'check', since it doesn't use variables such as missing mass. 

In the following example will use:

* Momentum
* `\delta t`
* Energy deposited
* Angles `\theta` and `\phi`
* `\delta E`
* `V_z`

---

### Methods

The code is setup so it's easy to compare the various MVA methods available

![roc0]( https://github.com/mj-will/ml4np/blob/master/figures/final/classifiers.png?raw=true)

---
