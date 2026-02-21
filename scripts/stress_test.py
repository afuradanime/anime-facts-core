# This is for the backend anime related endpoints, so theoretically this but not really :broken_heart:
import requests
import time
import random
import threading
from collections import defaultdict

BASE_URL = "http://localhost:6969"

ANIME_IDS     = [1, 7, 28, 95, 100, 411, 418, 11577, 11595, 45391]
STUDIO_IDS    = [1, 2, 3, 4, 5]
PRODUCER_IDS  = [1, 8, 3, 4, 5]
LICENSOR_IDS  = [15, 230, 321]
TAG_IDS       = [1, 2, 3, 4, 5, 6, 7, 8]
SEARCH_TERMS  = ["", "naruto", "one piece", "dragon", "sword", "attack"]

ENDPOINTS = [
    lambda: f"{BASE_URL}/anime/{random.choice(ANIME_IDS)}",
    lambda: f"{BASE_URL}/anime/random",
    lambda: f"{BASE_URL}/anime/search?name={random.choice(SEARCH_TERMS)}&page=0&pageSize=20",
    lambda: f"{BASE_URL}/anime/seasonal?page=0&pageSize=20",
    lambda: f"{BASE_URL}/anime/studio/{random.choice(STUDIO_IDS)}",
    lambda: f"{BASE_URL}/anime/producer/{random.choice(PRODUCER_IDS)}",
    lambda: f"{BASE_URL}/anime/licensor/{random.choice(LICENSOR_IDS)}",
    lambda: f"{BASE_URL}/anime/tags/{random.choice(TAG_IDS)}",
]

DURATION    = 60
WORKERS     = 10
INTERVAL    = 0.1  # seconds between requests per worker

# Shared counters
lock     = threading.Lock()
counts   = defaultdict(int)
errors   = defaultdict(int)
total    = 0
stop_evt = threading.Event()


def worker():
    global total
    session = requests.Session()
    while not stop_evt.is_set():
        url = random.choice(ENDPOINTS)()
        # Extract a readable endpoint name
        path = url.replace(BASE_URL, "").split("?")[0]

        try:
            r = session.get(url, timeout=5)
            is_err = r.status_code >= 500
        except Exception:
            is_err = True

        with lock:
            counts[path] += 1
            total += 1
            if is_err:
                errors[path] += 1

        time.sleep(INTERVAL)


threads = [threading.Thread(target=worker, daemon=True) for _ in range(WORKERS)]
start = time.time()

print(f"Running {WORKERS} workers for {DURATION}s against {BASE_URL}\n")
for t in threads:
    t.start()

while time.time() - start < DURATION:
    elapsed = time.time() - start
    print(f"\r  {elapsed:.0f}s / {DURATION}s  —  {total} requests", end="", flush=True)
    time.sleep(1)

stop_evt.set()
for t in threads:
    t.join()

elapsed = time.time() - start
print(f"\n\n{'─'*55}")
print(f"  {'ENDPOINT':<30} {'REQS':>6}  {'ERRS':>5}  {'ERR%':>5}")
print(f"{'─'*55}")
for path in sorted(counts):
    r = counts[path]
    e = errors[path]
    pct = e / r * 100 if r else 0
    print(f"  {path:<30} {r:>6}  {e:>5}  {pct:>4.1f}%")
print(f"{'─'*55}")
print(f"  {'TOTAL':<30} {total:>6}  {sum(errors.values()):>5}")
print(f"\n  Throughput: {total/elapsed:.1f} req/s")