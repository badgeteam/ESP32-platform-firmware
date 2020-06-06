import valuestore

def get(app_slug, default_config):
    config = valuestore.load(namespace='app', keyname=app_slug)
    if config is None:
        config = default_config
        valuestore.save(namespace='app', keyname=app_slug, value=default_config)
    return config