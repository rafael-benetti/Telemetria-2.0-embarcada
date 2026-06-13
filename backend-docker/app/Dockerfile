FROM node:20-alpine

WORKDIR /app
COPY server.js /app/server.js

ENV PORT=8080
ENV MQTT_HOST=mqtt
ENV MQTT_PORT=1883

EXPOSE 8080
CMD ["node", "server.js"]
