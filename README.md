# Online-fp-add
In this work we implement in C++ multi-term adders that interleave the maximum exponent calculation and fraction accumulation, for floating point addition that is used in dot-product units. We evaluate the area and power by performing reduction that occur in matrix multiplications from the inference of the BERT model using input from the GLUE dataset. More specifically, we run the pytorch model from [huggingface](https://huggingface.co/) and extracted inter layer results for the different tests included in [GLUE dataset](https://huggingface.co/datasets/nyu-mll/glue) to use as inputs in ```main.cc```.

The core framework of this project is the [Fast-Float4HLS](https://github.com/ic-lab-duth/Fast-Float4HLS.git) library, publicly available on github.

## Repository Hierarchy

This repository is organized as follows:

```bash
.
├── src
│   ├── file_io.h
│   ├── main_incl.h
│   ├── main.cc
│   ├── max_lzc.h
│   ├── multi_term_adders.h
│   └── norm_and_round.h
│
├── utils
│   ├── pack.py
│   └── run.py
│
├── .gitignore
├── LICENSE
├── README.md
└── setup.sh
```

* ```./src/``` This directory contains the C++ implementation of multi-term adders.
* ```./utils/``` This directory contains Python utility scripts.
* ```./setup.sh``` A bash script to fetch all required dependencies.

## Pending Features

* Add synthesis scripts.
* Python scripts for automatically loading and extracting BERT input on GLUE.
* Modify normalization and round logic to return a FP32 float.
* Integrade multi-term online adders onto dot-product units.

## Reference
This work was publish to IEEE Transaction on Very Large Scale Integration (VLSI) Systems. You can find the [paper](https://gdimitrak.github.io/papers/TVLSI24-Online-FLP-Add.pdf) here. To cite this work please use

@ARTICLE{10750002,
  author={Alexandridis, Kosmas and Dimitrakopoulos, Giorgos},
  journal={IEEE Transactions on Very Large Scale Integration (VLSI) Systems}, 
  title={Online Alignment and Addition in Multiterm Floating-Point Adders}, 
  year={2025},
  volume={33},
  number={4},
  pages={1182-1186},
  doi={10.1109/TVLSI.2024.3488966}}


## Contributors

Currently active: [Kosmas Alexandridis](https://github.com/kosmalex) and [Giorgos Dimitrakopoulos](https://github.com/gdimitrak)



## License

Online-fp-add is licensed with the MIT License. You are completely free to re-distribute your work derived from Online-fp-add
