<a name="readme-top"></a>

[![Contributors][contributors-shield]][contributors-url]
[![Stargazers][stars-shield]][stars-url]
[![MIT License][license-shield]][license-url]
[![LinkedIn][linkedin-shield]][linkedin-url]

<!-- PROJECT LOGO -->
<br />
<div align="center">
  <a href="https://github.com/FuryCode-bit/CryptoCat">
    <img src="readme/ua.png" alt="Logo" height="80">
  </a>

  <h3 align="center">CryptoCat: High-Performance Password Cracker</h3>

  <p align="center"> A C++/Python framework for generating and utilizing rainbow tables to crack hashed passwords.
    <br />
    <a href="https://github.com/FuryCode-bit/CryptoCat"><strong>Explore the docs »</strong></a>
    <br />
    <br />
    ·
    <a href="https://github.com/FuryCode-bit/CryptoCat/issues">Report Bug</a>
  </p>
</div>

<!-- ABOUT THE PROJECT -->
## About The Project

![Product Name Screen Shot][project-screenshot]

### Overview

CryptoCat is a robust framework designed for the efficient cracking of passwords using rainbow tables. It leverages a custom hashing scheme where passwords are expanded to a 16-byte (128-bit) key, which is then used in an AES-128 encryption to encrypt itself, producing the hashed password. The project consists of two primary C++ programs, `table` and `guess`, managed by a Python orchestrator (`main.py`), to handle the generation and lookup phases of rainbow table attacks.

The character set for passwords is defined as 64 characters: `[a-zA-Z0-9?!]`. The goal is to find a password `P` given its hash `H(P)` more efficiently than brute-force or pre-computed hash tables.

## Hashing Scheme

The project utilizes a specific password hashing scheme:
1.  **Password Expansion**: The input password is expanded by appending copies of itself until it reaches 16 bytes (128 bits) in length. This expanded string serves as both the AES-128 key and the plaintext.
    *   Example: `password = "abcdef"` expands to `key = "abdcefabcdefabcd"`
2.  **AES-128 Encryption**: The expanded key is then used with AES-128 (in ECB mode, no padding) to encrypt itself. The resulting ciphertext is the hashed password.
    *   `hashed_password = H(password) = AES_key(key)`

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- GETTING STARTED -->
## Getting Started

To get a local copy up and running, follow these simple steps.

### Prerequisites

*   C++ compiler (g++)
*   `make` utility
*   OpenSSL development libraries (e.g., `libssl-dev` on Debian/Ubuntu, `openssl-devel` on Fedora/RHEL)
*   Python 3.8+
*   `pip` for Python package management

### Installation

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/FuryCode-bit/CryptoCat.git
    cd CryptoCat
    ```

2.  **Compile the C++ executables:**
    ```bash
    make
    ```
    This will compile `table`, `guess`, `test_crypto`, and `create_single_chain` into the `bin/` directory.

3.  **Create a Python virtual environment and install dependencies:**
    ```bash
    python3 -m venv venv
    source venv/bin/activate  # On Windows, use `venv\Scripts\activate`
    pip install -r requirements.txt
    ```

### Execution

The main entry point for the CryptoCat framework is `main.py`, which orchestrates the C++ programs.

#### 1. Start the CryptoCat interactive menu:

```bash
python main.py
```
Or, to enable detailed logging:
```bash
python main.py --log=info
```

#### 2. Choose an Execution Mode:

You will be presented with a menu to select between `Single-Threaded Mode` and `Multi-Threaded Mode`.

#### 3. Select a Target:

Within your chosen mode, you can select one of the following targets:

*   **Crack a Single Hash**: You provide a single hash and its expected password length.
*   **Crack Hashes From a File**: You provide a file containing a list of hashes (one per line) and their common password length.
*   **Run Full Assignment**: This option attempts to crack a predefined list of target hashes (from the homework assignment) for various lengths. Results are stored in `data/assignment_cracked_passwords.txt`.

#### Example Usage (Conceptual via CLI for `table` and `guess`):

While `main.py` provides the interactive interface, here's how the underlying C++ programs would be called directly:

*   **Generate a Rainbow Table (`table`):**
    ```bash
    ./bin/table <password_length> <num_chains> <output_file.dat> [--threads N] [--verbose]
    ```
    Example: `./bin/table 6 1000000 data/rainbow_table_len6.dat --threads 4 --verbose`

*   **Guess a Password from a Hash using a Rainbow Table (`guess`):**
    ```bash
    ./bin/guess <rainbow_file.dat> <hash_hex> [--threads N] [--verbose]
    ```
    Example: `./bin/guess data/rainbow_table_len6.dat f8340c836d41f77cd92708bbd5443cbe --threads 4 --verbose`

#### Testing Utility (`test_crypto`):

To test the hashing mechanism for a specific password:
```bash
./bin/test_crypto <password_to_test>
```
Example: `./bin/test_crypto mypass`

#### Single Chain Creation Utility (`create_single_chain`):

To generate and verify a single chain for testing purposes:
```bash
./bin/create_single_chain <start_password> <password_len> <chain_len_k> <outfile>
```
Example: `./bin/create_single_chain abcde 5 10 data/single_chain.dat`

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- ROADMAP -->
## Roadmap

*   **Kubernetes Mode**: Implement a distributed cracking mode using Kubernetes for large-scale operations.
*   **GPU Acceleration**: Explore using CUDA/OpenCL for GPU-accelerated hash computations.

See the [open issues](https://github.com/FuryCode-bit/CryptoCat/issues) for a full list of proposed features (and known issues).

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- CONTRIBUTING -->
## Contributing

Contributions are what make the open source community such an amazing place to learn, inspire, and create. Any contributions you make are **greatly appreciated**.

If you have a suggestion that would make this better, please fork the repo and create a pull request. You can also open an issue with the tag "enhancement".
Don't forget to give the project a star! Thanks again!

1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- LICENSE -->
## License

Distributed under the MIT License. See `LICENSE` for more information.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->
[contributors-shield]: https://img.shields.io/github/contributors/FuryCode-bit/CryptoCat.svg?style=for-the-badge
[contributors-url]: https://github.com/FuryCode-bit/CryptoCat/graphs/contributors
[stars-shield]: https://img.shields.io/github/stars/FuryCode-bit/CryptoCat.svg?style=for-the-badge
[stars-url]: https://github.com/FuryCode-bit/CryptoCat/stargazers
[license-shield]: https://img.shields.io/github/license/FuryCode-bit/CryptoCat.svg?style=for-the-badge
[license-url]: https://github.com/FuryCode-bit/CryptoCat/blob/main/LICENSE
[linkedin-shield]: https://img.shields.io/badge/-LinkedIn-black.svg?style=for-the-badge&logo=linkedin&colorB=555
[linkedin-url]: https://linkedin.com/in/bernardeswebdev
[project-screenshot]: readme/cryptocat_screenshot.png
