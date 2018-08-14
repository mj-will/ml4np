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

Tested the avaiable methods in TMVA and found Boosted Decision Trees (BDTs) to be the best suited for this problem. 


