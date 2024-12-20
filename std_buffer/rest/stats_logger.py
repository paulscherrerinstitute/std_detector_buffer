import logging
from threading import Event, Thread
from time import sleep, time

_logger = logging.getLogger("StatsLogger")


class EventFilter(logging.Filter):
    def filter(self, record):
        record.event = "[event]"
        return True


_logger.addFilter(EventFilter())


class StatsLogger:
    def __init__(self, ctx=None):
        self.ctx = ctx

        self._stop_event = Event()
        self.config_stats = []
        self._stats_thread = Thread(target=self._collect_stats)
        self._stats_thread.start()

    def _collect_stats(self):
        while not self._stop_event.is_set():
            try:
                sleep(1)
                # Process stats if necessary (currently just sleeps)
            except Exception:
                _logger.exception("Error in stats loop.")
                sleep(1)

    def log_config_change(self, action, user, success):
        timestamp = time()
        log_entry = {
            "action": action,
            "user": user,
            "timestamp": timestamp,
            "success": success,
        }
        self.config_stats.append(log_entry)
        _logger.info(f"Logged config activity: {log_entry}")

    def close(self):
        self._stop_event.set()
        self._stats_thread.join()
