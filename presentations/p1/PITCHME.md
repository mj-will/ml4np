##  Progress meeting 11/06/18

What have I done?
- Learn ROOT and TMVA
- Initial tests and comparisons on using Keras with ROOT and TMVA with simpler dataset
- Get a system setup that we can use with PyTMVA, Keras and Tensorflow (NP-Phi) 
---

### ROOT, TMVA and Python

Possible workflow is complex neural nets were used:
- Setup network and train network in python using keras and pyROOT
- Apply network to data in ROOT using TMVA::Reader Class which can take 'PyKeras' type classifier:

```
reader->BookMVA("PyKeras", TString("dataset/weights/TMVAClassification_PyKeras.weights.       xml"));
```

This allows for more complex networks to be developed in Python, but they can be applied directly in a ROOT macro.

---

### Initial tests

![fig1](figures/comparison1.pdf)


---

### System setup - NP-Phi

NP-Phi has been setup with the nessary versions of ROOT, TMVA and the correct python packages so all the methods mentioned can be used.

I've kept track of the issues in a [wiki page](https://github.com/mj-will/ml4np/wiki/PyROOT-Issues-&-Observations)

Only setup required for a user on the system is changing their `.cshrc` file.
