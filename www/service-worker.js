// The cache name should change every time you want to "cache bust"
const CACHE_NAME = "nuclear-chess-v1";
const urlsToCache = [
  "css/bootstrap.min.css",
  "css/default.css",
  "css/fonts/rock-salt-v6-latin-regular.woff",
  "css/fonts/rock-salt-v6-latin-regular.woff2",
  "js/atomschach.js",
  "js/bootstrap.min.js",
  "js/game.js",
  "js/jquery.min.js",
  "js/main.js",
  "lib/chessboardjs/css/chessboard-0.3.0.css",
  "lib/chessboardjs/css/chessboard-0.3.0.min.css",
  "lib/chessboardjs/img/chesspieces/wikipedia/bB.png",
  "lib/chessboardjs/img/chesspieces/wikipedia/bK.png",
  "lib/chessboardjs/img/chesspieces/wikipedia/bN.png",
  "lib/chessboardjs/img/chesspieces/wikipedia/bP.png",
  "lib/chessboardjs/img/chesspieces/wikipedia/bQ.png",
  "lib/chessboardjs/img/chesspieces/wikipedia/bR.png",
  "lib/chessboardjs/img/chesspieces/wikipedia/nuke.png",
  "lib/chessboardjs/img/chesspieces/wikipedia/wB.png",
  "lib/chessboardjs/img/chesspieces/wikipedia/wK.png",
  "lib/chessboardjs/img/chesspieces/wikipedia/wN.png",
  "lib/chessboardjs/img/chesspieces/wikipedia/wP.png",
  "lib/chessboardjs/img/chesspieces/wikipedia/wQ.png",
  "lib/chessboardjs/img/chesspieces/wikipedia/wR.png",
  "lib/chessboardjs/js/chessboard.js",
];

/**
 * Listen for the install event, which fires when the service worker is installing.
 * We use event.waitUntil() to ensure the install doesn't finished until our promise resolves
 * so we don't do anything else until the initial caching is done.
 */
self.addEventListener("install", async (event) => {
  console.log("installing!");
  self.skipWaiting();
  event.waitUntil(cache_assets());
});

async function cache_assets() {
  const cache = await self.caches.open(CACHE_NAME);
  return cache.addAll(urlsToCache);
}

/**
 * Listen for the activate event, which is fired after installation
 * Activate is when the service worker actually takes over from the previous
 * version, which is a good time to clean up old caches.
 * Again we use waitUntil() to ensure we don't move on until the old caches are deleted.
 */
self.addEventListener("activate", async (event) => {
  console.log("activating!");
  event.waitUntil(delete_old_caches());
});

async function delete_old_caches() {
  // Get the keys of all the old caches
  const keys = await caches.keys();
  const deletePromises = keys
    .filter((key) => key !== CACHE_NAME)
    .map((key) => self.caches.delete(key));
  return Promise.all(deletePromises);
}

/**
 * Listen for browser fetch events.
 * These fire any time the browser tries to load anything.
 * This isn't just fetch() calls; clicking a <a href> triggers it too.
 */
self.addEventListener("fetch", (event) => {
  console.log("fetching!");
  event.respondWith(get_response(event.request));
});

/**
 * We follow the "stale-while-revalidate" pattern:
 * respond with the cached response immediately (if we have one)
 * even though this might be "stale" (not the most recent version).
 * In the background fetch the latest version and put that into cache
 * on next request the user will get the latest version
 */
async function get_response(request) {
  const cache = await self.caches.open(CACHE_NAME);
  const cached_response = await cache.match(request);
  // Important: we do not await here, since that would defeat the point of using the cache
  const pending_response = fetch(request).then((response) => {
    cache.put(request, response.clone());
    return response;
  });
  return cached_response || pending_response;
}
