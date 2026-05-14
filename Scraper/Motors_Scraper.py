from selenium import webdriver
from selenium.webdriver.chrome.service import Service
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC
from bs4 import BeautifulSoup
import json
import pandas as pd
import random
import time

# Path to ChromeDriver executable
driver_path = 'C:/webdriver/chromedriver.exe'

# Create Selenium Chrome service and browser options
session = Service(driver_path)
option = webdriver.ChromeOptions()
# option.add_argument('--headless')  # Run browser in background (no UI)

# Initialize WebDriver and explicit wait helper
driver = webdriver.Chrome(service=session, options=option)
wait = WebDriverWait(driver, 10)



def GetProductsUrls():
    """
    Iterate over all pages, extract each product URL and return them as a list.
    """
    data = []

    for page_number in range(1, 266):
        while True:
            try:
                driver.get(
                    f"https://www.maxongroup.com/maxon/view/service_search?query=*&filterKategorie1=Motor&channel=factfinderProducts_ws_maxon_com__en&page={page_number}&followSearch=10000&format=JSON"
                )

                wait.until(EC.presence_of_element_located((By.XPATH, '//table/tbody/tr')))
                time.sleep(random.uniform(1, 3))

                page_source = driver.page_source
                page = BeautifulSoup(page_source, 'lxml')
                table = page.find('table')
                if not table or not table.find('tbody'):
                    print(f'No table on page {page_number}')
                    break

                rows = table.find('tbody').find_all('tr')
                for row in rows:
                    try:
                        tds = row.find_all('td')
                        data_row = tds[2].find('ul').find_all('li')
                        diameter = data_row[1].find_all('span')[1].get_text(strip=True)
                        width = data_row[2].find_all('span')[1].get_text(strip=True)
                        url_suffix = tds[3].find('ul').find('li').find('a')['href']
                        data.append(("https://www.maxongroup.com" + url_suffix, diameter, width))
                    except Exception as e:
                        print(f"Skipping a row on page {page_number}: {e}")
                break
            except Exception as e:
                print(f'Error on page {page_number}: {e}')

    with open('motorsUrls.txt', 'w', encoding='utf-8') as f:
        for url, diameter, width in data:
            f.write(f"{url}\t{diameter}\t{width}\n")
    return data


def GetProductsDetails(data):
    """
    get each product page, extract details,
    and save the specifications to motorsDetails.json.
    """
    specifications = []

    for index, (url, diameter, width) in enumerate(data):
        counter = 0
        specification = None

        while counter < 10:
            try:
                driver.get(url)
                wait.until(EC.presence_of_element_located((By.CLASS_NAME, 'iTabSpecifications')))
                page_source = driver.page_source
                soup = BeautifulSoup(page_source, 'lxml')
                spec_div = soup.find('div', class_='iTabSpecifications')
                tables = spec_div.find_all('tbody') if spec_div else []

                specification = {
                    'index': index,
                    'url': url.strip(),
                    'diameter': diameter,
                    'width': width
                }

                for tbody in tables:
                    items = tbody.find_all('td')
                    for j in range(0, len(items) - 1, 2):
                        key = items[j].get_text(strip=True)
                        value = items[j + 1].get_text(strip=True)
                        specification[key] = value

                break

            except Exception as e:
                counter += 1
                print(f"Error processing line {index}: {e}")

        if specification is None:
            print(f"Skipping line {index} after 10 failed attempts.")
            continue

        specifications.append(specification)

    with open('motorsDetails.json', 'w', encoding='utf-8') as file:
        json.dump(specifications, file, indent=4, ensure_ascii=False)



def convert_json_to_csv():
    """
    Load JSON data from motorsDetails.json and export to CSV.
    """
    with open('motorsDetails.json', 'r', encoding='utf-8') as file:
        data = json.load(file)

    df = pd.json_normalize(data)
    # df = df.reindex(columns=keys)  # Ensure all keys are included as columns
    df.columns = df.columns.str.strip()
    df.fillna('N/A', inplace=True)  # Replace NaN with empty strings
    df.to_csv('motorsDetails.csv', index=False, encoding='utf-8')

def CleanData():
    """
    Load the CSV file, clean the data by removing unwanted characters,
    and save the cleaned data back to CSV.
    """
    df = pd.read_csv('motorsDetails.csv', encoding='utf-8')
    # Define common suffixes to remove and rename columns accordingly
    suffixes = [
        ' Nm', ' mm', ' %', ' : 1', ' N', ' rpm', ' g',
        ' gcm²', ' W', ' mm/s', ' °C', ' °', ' V', ' A',
        ' mA', ' mNm', ' Ω', ' mH', ' mNm/A', ' rpm/V',
        ' rpm/mNm', ' ms', ' K/W', ' s'
                ]
    # Remove unwanted characters from all string columns
    for column in df.select_dtypes(include=['object']).columns:
        df[column] = df[column].str.removesuffix(' ')
        for suffix in suffixes:
            if df[column].str.endswith(suffix).any():
                df[column] = df[column].str.removesuffix(suffix)
                df.rename(columns={column: f'{column} ({suffix.replace(" ", "")})'}, inplace=True) # Rename column to include suffix in parentheses without spaces
                column = f'{column} ({suffix.replace(" ", "")})'  # Update column name for next iterations

        df[column] = df[column].str.removeprefix('max. ')
    df.to_csv('motorsDetailsCleaned.csv', index=False, encoding='utf-8')




#add index colomn at first of csv file
def AddIndexColumn():
    df = pd.read_csv('motorsDetailsCleaned.csv', encoding='utf-8')
    df.insert(0, 'Index', range(0, len(df)))  # Add index column starting from 0
    df.to_csv('motorsDetailsCleanedIndexed.csv', index=False, encoding='utf-8')