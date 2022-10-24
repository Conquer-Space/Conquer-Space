import requests
content = requests.get("https://celestrak.org/NORAD/elements/active.txt", stream=True).content
with open('satellites.txt', 'wb') as out_file:
    out_file.write(content)
