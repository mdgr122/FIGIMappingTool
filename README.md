# FIGI Mapping Tool

A lightweight tool for mapping equity identifiers to FIGI codes in bulk using the [OpenFIGI API](https://www.openfigi.com/api). You can use your own API key by replacing the default one at the bottom of the application window.

## Table of Contents

- [Features](#features)
- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [Usage](#usage)
  - [Input File Format](#input-file-format)
  - [Running the Tool](#running-the-tool)
  - [Output Formats](#output-formats)
- [License](#license)
- [Acknowledgements](#acknowledgements)
- [Disclaimer](#disclaimer)
- [Support](#support)

## Features

- **Bulk Mapping**: Efficiently map large lists of equity identifiers to FIGI codes.
- **Flexible Input**: Supports various base identifiers and optional context values.
- **Multiple Output Formats**: Obtain results in Raw JSON, Raw Text, or Parsed CSV.
- **Customizable API Key**: Easily replace the default API key with your own for higher rate limits.

## Prerequisites

- **Operating System**: Windows 7 or higher.
- **Dependencies**: Ensure that you have the required libraries and dependencies installed if you're compiling from source.
  - **Visual Studio**: Recommended for compiling the source code.
  - **OpenFIGI API Key**: Obtain one [here](https://www.openfigi.com/api) (optional but recommended).

## Installation

1. **Download the Latest Release**: Get the executable from the [releases page](https://github.com/mdgr122/FIGIMappingTool/releases).

2. **Optional - Compile from Source**:
   - Clone the repository:
     ```bash
     git clone https://github.com/mdgr122/FIGIMappingTool.git
     ```
   - Open the solution file in Visual Studio.
   - Build the project to generate the executable.

3. **Obtain an OpenFIGI API Key** *(Optional but Recommended)*:
   - Sign up for an API key at the [OpenFIGI API portal](https://www.openfigi.com/api).

## Usage

### Input File Format

The input file should be a plain text file (`*.txt`) containing a list of identifiers. Each line includes a base identifier and an optional context value, separated by a space.

#### Base Identifiers

- **TICKER**
- **ISIN**
- **CUSIP**
- **SEDOL**
- **COMPOSITE_FIGI**

#### Optional Context Values

- **Currency**
- **MIC Code**
- **Bloomberg Exchange Code**

> **Note**: When searching for an **index**, you must append `" Index"` to the index ticker. This is how the OpenFIGI API labels equity indices.

#### Sample Input File (`input.txt`)
AAPL XNGS 
AAPL UW 
CAR.UN 
SPX Index 
TSLA
US0378331005
US88160R1014 UN


### Running the Tool

1. **Launch the Application**: Double-click the executable to start the tool.

2. **Select Input File**:
   - Click the **File** button to browse and select your input text file.

3. **Specify Output Path**:
   - Click the **Save** button to choose the destination and name for your output file.
   - Use the **:** button to specify the save path.

4. **Enter API Key** *(Optional)*:
   - Replace the default API key at the bottom of the application window with your own if you have one.

5. **Execute the Mapping**:
   - Click the **REQUEST** button to start the mapping process.
   - The status messages will update to reflect the progress.

6. **Save the Results**:
   - Once processing is complete, click the **Save** button to output the results in your chosen format.

### Output Formats

- **Raw JSON**: The unprocessed JSON response directly from the OpenFIGI API.
- **Raw Text**: A plain text version of the response.
- **Parsed CSV**: A CSV file with the parsed and structured results for easy analysis in spreadsheet applications.

## License

This project is licensed under the [MIT License](LICENSE).

### OpenFIGI API License

The OpenFIGI API is provided by Bloomberg Finance L.P. under the [Apache License 2.0](http://www.apache.org/licenses/LICENSE-2.0). By using this tool, you agree to comply with the OpenFIGI API [Terms of Use](https://www.openfigi.com/api#terms).

---

*Please ensure that you have read and understood the OpenFIGI API terms and conditions. It is your responsibility to comply with all applicable licensing requirements.*

## Acknowledgements

- **OpenFIGI API**: This project utilizes the OpenFIGI API provided by Bloomberg Finance L.P.

## Disclaimer

This tool is an independent project and is not affiliated with, endorsed by, or associated with Bloomberg Finance L.P. or OpenFIGI. The use of the OpenFIGI API is subject to their [Terms of Use](https://www.openfigi.com/api#terms).

## Support

For any issues or feature requests, please open an issue on the [GitHub repository](https://github.com/mdgr122/FIGIMappingTool/issues).

---
