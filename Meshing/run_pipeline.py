try:
    from pipeline.cli import main
except ImportError:
    from Meshing.pipeline.cli import main


if __name__ == "__main__":
    raise SystemExit(main())
