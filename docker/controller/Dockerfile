# Use Ubuntu as the base image
FROM ubuntu:22.04

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive
ENV PYTHONUNBUFFERED=1

# Install Python and other necessary packages
RUN apt-get update && \
    apt-get install -y \
        python3 \
        python3-pip \
        python3-venv \
        build-essential \
        git \
        && apt-get clean && rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /app

# Copy the pyproject.toml and poetry.lock (if available)
COPY pyproject.toml ./

# Install Poetry
RUN pip3 install --no-cache-dir poetry

# Install dependencies
RUN poetry install --no-dev

# Copy the rest of the application code
COPY ./src ./src

# Set the working directory to the location of main.py
WORKDIR /app/src/

# Specify the command to run your application
CMD ["python3", "main.py"]
