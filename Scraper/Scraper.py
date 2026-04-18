from selenium import webdriver
from selenium.webdriver.chrome.service import Service
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC
from bs4 import BeautifulSoup

driver_path = 'C:/webdriver/chromedriver.exe'
session = Service(driver_path)
option = webdriver.ChromeOptions()
option.add_argument('--headless')
driver = webdriver.Chrome(service=session, options=option)
wait = WebDriverWait(driver, 10)
driver.get("https://www.maxongroup.com/maxon/view/category/prodfilter/motor?pn_id=ProductSearch&pn_p=1")
wait.until(lambda d: d.execute_script("""
    return document.readyState === 'complete'
"""))
page_source = driver.page_source
soup = BeautifulSoup(page_source,'lxml')
table = soup.find('table').find('tbody').find_all('tr')
print(table[0]['data-detailurl'])